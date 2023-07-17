import TransactionDescriptorProcessor from '../src/TransactionDescriptorProcessor.js';
import { expect } from 'chai';

describe('TransactionDescriptorProcessor', () => {
	// region test utils

	const createProcessor = extendedDescriptor => {
		const transactionDescriptor = {
			type: 'transfer',
			timestamp: 12345,
			signer: 'signerName',
			recipient: 'recipientName',
			message: 'hello world',
			...(extendedDescriptor || {})
		};
		const typeParsingRules = new Map();
		typeParsingRules.set('PublicKey', name => `${name} PUBLICKEY`);

		const processor = new TransactionDescriptorProcessor(transactionDescriptor, typeParsingRules);
		processor.setTypeHints({ signer: 'PublicKey', timestamp: 'Number' });
		return processor;
	};

	const createProcessorWithConverter = deadlineValue => {
		const transactionDescriptor = {
			type: 'transfer',
			timestamp: 12345,
			signer: 'signerName',
			recipient: 'recipientName',
			message: 'hello world',
			fee: 100,
			deadline: undefined === deadlineValue ? 300 : deadlineValue
		};
		const typeParsingRules = new Map();
		typeParsingRules.set('Number', value => value + 42);

		const typeConverter = value => ('number' === typeof value ? value * 2 : value);
		const processor = new TransactionDescriptorProcessor(transactionDescriptor, typeParsingRules, typeConverter);
		processor.setTypeHints({ timestamp: 'Number' });
		return processor;
	};

	// endregion

	// region lookupValue

	describe('lookupValue', () => {
		const assertCannotLookupValueWhenDescriptorDoesNotContainKey = processorFactory => {
			// Arrange:
			const processor = processorFactory();

			// Act + Assert:
			expect(() => { processor.lookupValue('foo'); }).to.throw('transaction descriptor does not have attribute');
		};

		it('cannot lookup value when descriptor does not contain key', () => {
			assertCannotLookupValueWhenDescriptorDoesNotContainKey(createProcessor);
		});

		it('cannot lookup value when descriptor does not contain key (with converter)', () => {
			assertCannotLookupValueWhenDescriptorDoesNotContainKey(createProcessorWithConverter);
		});

		const assertCanLookupValueMessage = processorFactory => {
			// Arrange:
			const processor = processorFactory();

			// Act:
			const message = processor.lookupValue('message');

			// Assert:
			expect(message).to.equal('hello world');
		};

		it('can lookup value without type hint', () => {
			assertCanLookupValueMessage(createProcessor);
		});

		it('can lookup value without conversion', () => {
			assertCanLookupValueMessage(createProcessorWithConverter);
		});

		it('can lookup value with type hint but without custom rule', () => {
			// Arrange:
			const processor = createProcessor();

			// Act:
			const timestamp = processor.lookupValue('timestamp');

			// Assert:
			expect(timestamp).to.equal(12345);
		});

		it('can lookup value when hints are applied before conversion', () => {
			// Arrange:
			const processor = createProcessorWithConverter();

			// Act:
			const timestamp = processor.lookupValue('timestamp');

			// Assert: (12345 + 42) * 2
			expect(timestamp).to.equal(24774);
		});

		it('can lookup value with type hint and with custom rule', () => {
			// Arrange:
			const processor = createProcessor();

			// Act:
			const signer = processor.lookupValue('signer');

			// Assert:
			expect(signer).to.equal('signerName PUBLICKEY');
		});

		it('can lookup value when applying converter to all fields', () => {
			// Arrange:
			const processor = createProcessorWithConverter();

			// Act:
			const fee = processor.lookupValue('fee');
			const deadline = processor.lookupValue('deadline');

			// Assert:
			expect(fee).to.equal(200);
			expect(deadline).to.equal(600);
		});

		it('can lookup value when applying converter to all array elements', () => {
			// Arrange: specify the deadline value as an array
			const processor = createProcessorWithConverter([100, 300, 600]);

			// Act:
			const deadline = processor.lookupValue('deadline');

			// Assert:
			expect(deadline).to.deep.equal([200, 600, 1200]);
		});

		it('can lookup value with zero value', () => {
			// Arrange: specify the deadline value as zero
			const processor = createProcessorWithConverter(0);

			// Act:
			const deadline = processor.lookupValue('deadline');

			// Assert:
			expect(deadline).to.equal(0);
		});
	});

	// endregion

	// region copyTo

	describe('copyTo', () => {
		it('cannot copy to when descriptor contains fields not in transaction', () => {
			// Arrange:
			const processor = createProcessor();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null // missing message
			};

			// Act + Assert:
			expect(() => { processor.copyTo(transaction); }).to.throw('transaction does not have attribute');
		});

		it('cannot copy when descriptor contains computed field', () => {
			// Arrange:
			const processor = createProcessor({ messageEnvelopeSizeComputed: 123 });
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null
			};

			// Act + Assert:
			expect(() => { processor.copyTo(transaction); }).to.throw('cannot explicitly set computed field');
		});

		it('can copy to when transaction contains exact fields in descriptor', () => {
			// Arrange:
			const processor = createProcessor();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null
			};

			// Act:
			processor.copyTo(transaction);

			// Assert:
			expect(transaction).to.deep.equal({
				type: 'transfer',
				timestamp: 12345,
				signer: 'signerName PUBLICKEY',
				recipient: 'recipientName',
				message: 'hello world'
			});
		});

		it('can copy to when transaction contains fields not in descriptor', () => {
			// Arrange:
			const processor = createProcessor();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null, foo: null
			};

			// Act:
			processor.copyTo(transaction);

			// Assert:
			expect(transaction).to.deep.equal({
				type: 'transfer',
				timestamp: 12345,
				signer: 'signerName PUBLICKEY',
				recipient: 'recipientName',
				message: 'hello world',
				foo: null
			});
		});

		it('can copy to when ignore keys is not empty', () => {
			// Arrange:
			const processor = createProcessor();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null
			};

			// Act:
			processor.copyTo(transaction, ['type', 'recipient']);

			// Assert:
			expect(transaction).to.deep.equal({
				type: null,
				timestamp: 12345,
				signer: 'signerName PUBLICKEY',
				recipient: null,
				message: 'hello world'
			});
		});

		it('can copy to when transaction contains iterable attribute', () => {
			// Arrange:
			const transactionDescriptor = {
				type: 'transfer',
				signer: 'signerName',
				mosaics: [[1, 2], [3, 5]]
			};
			const typeParsingRules = new Map();
			typeParsingRules.set('PublicKey', name => `${name} PUBLICKEY`);

			const processor = new TransactionDescriptorProcessor(transactionDescriptor, typeParsingRules);
			processor.setTypeHints({ signer: 'PublicKey' });

			const transaction = {
				type: null, signer: null, mosaics: []
			};

			// Act:
			processor.copyTo(transaction);

			// Assert:
			expect(transaction).to.deep.equal({
				type: 'transfer',
				signer: 'signerName PUBLICKEY',
				mosaics: [[1, 2], [3, 5]]
			});
		});

		// NOTE: tuple and bytes test from python are inapplicable

		it('can copy to with custom converter', () => {
			// Arrange:
			const processor = createProcessorWithConverter();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null, fee: null, deadline: null
			};

			// Act:
			processor.copyTo(transaction);

			// Assert:
			expect(transaction).to.deep.equal({
				type: 'transfer',
				timestamp: 24774,
				signer: 'signerName',
				recipient: 'recipientName',
				message: 'hello world',
				fee: 200,
				deadline: 600
			});
		});
	});

	// endregion

	// region setTypeHints

	describe('setTypeHints', () => {
		it('can change type hints', () => {
			// Arrange:
			const processor = createProcessor();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null
			};

			// Act:
			processor.setTypeHints({ recipient: 'PublicKey' });
			processor.copyTo(transaction);

			// Assert:
			expect(transaction).to.deep.equal({
				type: 'transfer',
				timestamp: 12345,
				signer: 'signerName',
				recipient: 'recipientName PUBLICKEY',
				message: 'hello world'
			});
		});

		it('can clear type hints', () => {
			// Arrange:
			const processor = createProcessor();
			const transaction = {
				type: null, timestamp: null, signer: null, recipient: null, message: null
			};

			// Act:
			processor.setTypeHints(undefined);
			processor.copyTo(transaction);

			// Assert:
			expect(transaction).to.deep.equal({
				type: 'transfer',
				timestamp: 12345,
				signer: 'signerName',
				recipient: 'recipientName',
				message: 'hello world'
			});
		});
	});

	// endregion
});
