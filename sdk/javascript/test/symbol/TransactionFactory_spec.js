const { Hash256, PublicKey } = require('../../src/CryptoTypes');
const { Address, Network } = require('../../src/symbol/Network');
const { TransactionFactory } = require('../../src/symbol/TransactionFactory');
const { generateMosaicId, generateNamespaceId } = require('../../src/symbol/idGenerator');
const sc = require('../../src/symbol/models');
const { uint8ToHex } = require('../../src/utils/converter');
const { runBasicTransactionFactoryTests } = require('../test/basicTransactionFactoryTests');
const { expect } = require('chai');
const crypto = require('crypto');

describe('transaction factory (Symbol)', () => {
	const TEST_SIGNER_PUBLIC_KEY = new PublicKey(crypto.randomBytes(PublicKey.SIZE));

	const assertTransfer = transaction => {
		expect(transaction.type).to.equal(sc.TransactionType.TRANSFER);
		expect(transaction.version).to.equal(1);
		expect(transaction.network).to.deep.equal(sc.NetworkType.TESTNET);
	};

	// region rules

	it('has rules with expected hints', () => {
		// Act:
		const factory = new TransactionFactory(Network.TESTNET);

		// Assert:
		const expectedRuleNames = new Set([
			'Amount', 'BlockDuration', 'BlockFeeMultiplier', 'Difficulty', 'FinalizationEpoch', 'FinalizationPoint', 'Height', 'Importance',
			'ImportanceHeight', 'MosaicId', 'MosaicNonce', 'MosaicRestrictionKey', 'NamespaceId', 'Timestamp',
			'UnresolvedMosaicId',

			'MosaicFlags', 'AccountRestrictionFlags',

			'AliasAction', 'LinkAction', 'LockHashAlgorithm',
			'MosaicRestrictionType', 'MosaicSupplyChangeAction',
			'NamespaceRegistrationType', 'NetworkType', 'TransactionType',

			'struct:UnresolvedMosaic',

			'UnresolvedAddress', 'Address', 'Hash256', 'PublicKey', 'VotingPublicKey',

			'array[UnresolvedMosaicId]', 'array[TransactionType]', 'array[UnresolvedAddress]', 'array[UnresolvedMosaic]'
		]);
		expect(new Set(Array.from(factory.factory.rules.keys()))).to.deep.equal(expectedRuleNames);
	});

	// endregion

	const runSymbolTransactionFactoryTests = testDescriptor => {
		// region create

		it('can create known transaction with multiple overrides', () => {
			// Arrange:
			const typeRuleOverrides = new Map();
			typeRuleOverrides.set(Hash256, x => `${x} a hash`);
			typeRuleOverrides.set(sc.BlockDuration, () => 654321);
			typeRuleOverrides.set(PublicKey, x => `${x} PUBLICKEY`);
			const factory = testDescriptor.createFactory(typeRuleOverrides);

			// Act:
			const transaction = testDescriptor.createTransaction(factory)({
				type: 'hash_lock_transaction',
				signerPublicKey: 'signerName',
				hash: 'not really',
				duration: 'fake duration',
				mosaic: { mosaicId: 0x12345678ABCDEFn, amount: 12345n }
			});

			// Assert:
			expect(transaction.type).to.equal(sc.TransactionType.HASH_LOCK);
			expect(transaction.version).to.equal(1);
			expect(transaction.network).to.deep.equal(sc.NetworkType.TESTNET);

			const stringToBytes = value => new TextEncoder().encode(value);
			expect(transaction.signerPublicKey).to.deep.equal(stringToBytes('signerName PUBLICKEY'));
			expect(transaction.duration).to.equal(654321);
			expect(transaction.hash).to.deep.equal(stringToBytes('not really a hash'));
			expect(transaction.mosaic.mosaicId).to.deep.equal(new sc.UnresolvedMosaicId(0x12345678ABCDEFn));
			expect(transaction.mosaic.amount).to.deep.equal(new sc.Amount(12345n));
		});

		// endregion

		// region address type conversion

		it('can create transaction with address', () => {
			// Arrange: this tests the custom type converter
			const factory = testDescriptor.createFactory();

			// Act:
			const transaction = testDescriptor.createTransaction(factory)({
				type: 'account_address_restriction_transaction',
				signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
				restrictionAdditions: [
					new Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA'),
					new Address('DINRYHI6D4QCCIRDEQSSMJZIFEVCWLBNFYXTAMI')
				]
			});

			// Assert:
			const range = (start, end) => new Uint8Array(end - start).fill().map((_, i) => start + i);
			expect(transaction.restrictionAdditions).to.deep.equal([
				new sc.UnresolvedAddress(range(1, 25)), new sc.UnresolvedAddress(range(26, 50))
			]);
		});

		// endregion

		// region id autogeneration

		it('can autogenerate namespace registration root id', () => {
			// Arrange:
			const factory = testDescriptor.createFactory();

			// Act:
			const transaction = testDescriptor.createTransaction(factory)({
				type: 'namespace_registration_transaction',
				signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
				registrationType: 'root',
				duration: 123n,
				name: 'roger'
			});

			// Assert:
			const expectedId = generateNamespaceId('roger');
			expect(transaction.id.value).to.equal(expectedId);
		});

		it('can autogenerate namespace registration child id', () => {
			// Arrange:
			const factory = testDescriptor.createFactory();

			// Act:
			const transaction = testDescriptor.createTransaction(factory)({
				type: 'namespace_registration_transaction',
				signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
				registrationType: 'child',
				parentId: generateNamespaceId('roger'),
				name: 'charlie'
			});

			// Assert:
			const expectedId = generateNamespaceId('charlie', generateNamespaceId('roger'));
			expect(transaction.id.value).to.equal(expectedId);
		});

		it('can autogenerate mosaic definition id', () => {
			// Arrange:
			const factory = testDescriptor.createFactory();

			// Act:
			const transaction = testDescriptor.createTransaction(factory)({
				type: 'mosaic_definition_transaction',
				signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
				nonce: 123
			});

			// Assert:
			const expectedId = generateMosaicId(factory.network.publicKeyToAddress(new PublicKey(TEST_SIGNER_PUBLIC_KEY)), 123);
			expect(transaction.id.value).to.equal(expectedId);
		});

		// endregion
	};

	describe('transaction', () => {
		const testDescriptor = {
			name: 'Transaction',
			createFactory: typeRuleOverrides => new TransactionFactory(Network.TESTNET, typeRuleOverrides),
			createTransaction: factory => (descriptor => factory.create(descriptor)),
			assertTransaction: assertTransfer,
			assertSignature: (transaction, signature, signedTransactionPayload) => {
				const transactionHex = uint8ToHex(transaction.serialize());
				const expectedJsonString = `{"payload": "${transactionHex}"}`;
				expect(signedTransactionPayload).to.equal(expectedJsonString);
			}
		};
		runBasicTransactionFactoryTests(testDescriptor);
		runSymbolTransactionFactoryTests(testDescriptor);
	});

	describe('embedded transaction', () => {
		const testDescriptor = {
			name: 'EmbeddedTransaction',
			createFactory: typeRuleOverrides => new TransactionFactory(Network.TESTNET, typeRuleOverrides),
			createTransaction: factory => (descriptor => factory.createEmbedded(descriptor)),
			assertTransaction: assertTransfer
		};
		runBasicTransactionFactoryTests(testDescriptor, false);
		runSymbolTransactionFactoryTests(testDescriptor);
	});
});
