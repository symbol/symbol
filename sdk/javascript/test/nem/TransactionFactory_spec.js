const { PublicKey, Signature } = require('../../src/CryptoTypes');
const { Address, Network } = require('../../src/nem/Network');
const { TransactionFactory } = require('../../src/nem/TransactionFactory');
const nc = require('../../src/nem/models');
const { uint8ToHex } = require('../../src/utils/converter');
const { runBasicTransactionFactoryTests } = require('../test/basicTransactionFactoryTests');
const { expect } = require('chai');
const crypto = require('crypto');

describe('transaction factory (NEM)', () => {
	const TEST_SIGNER_PUBLIC_KEY = new PublicKey(crypto.randomBytes(PublicKey.SIZE));

	const assertTransfer = transaction => {
		expect(transaction.type).to.equal(nc.TransactionType.TRANSFER);
		expect(transaction.version).to.equal(2);
		expect(transaction.network).to.deep.equal(nc.NetworkType.TESTNET);
	};

	const testDescriptor = {
		name: 'Transaction',
		createFactory: typeRuleOverrides => new TransactionFactory(Network.TESTNET, typeRuleOverrides),
		createTransaction: factory => (descriptor => factory.create(descriptor)),
		assertTransaction: assertTransfer,
		assertSignature: (transaction, signature, signedTransactionPayload) => {
			const transactionHex = uint8ToHex(TransactionFactory.toNonVerifiableTransaction(transaction).serialize());
			const signatureHex = signature.toString();
			const expectedJsonString = `{"data":"${transactionHex}", "signature":"${signatureHex}"}`;
			expect(signedTransactionPayload).to.equal(expectedJsonString);
		}
	};

	runBasicTransactionFactoryTests(testDescriptor);

	// region rules

	it('has rules with expected hints', () => {
		// Act:
		const factory = new TransactionFactory(Network.TESTNET);

		// Assert:
		const expectedRuleNames = new Set([
			'Amount', 'Height', 'Timestamp',

			'LinkAction', 'MessageType', 'MosaicSupplyChangeAction', 'MosaicTransferFeeType',
			'MultisigAccountModificationType', 'NetworkType', 'TransactionType',

			'struct:Message', 'struct:NamespaceId', 'struct:MosaicId', 'struct:Mosaic', 'struct:SizePrefixedMosaic', 'struct:MosaicLevy',
			'struct:MosaicProperty', 'struct:SizePrefixedMosaicProperty', 'struct:MosaicDefinition',
			'struct:MultisigAccountModification', 'struct:SizePrefixedMultisigAccountModification',

			'Address', 'Hash256', 'PublicKey',

			'array[SizePrefixedMosaic]', 'array[SizePrefixedMosaicProperty]', 'array[SizePrefixedMultisigAccountModification]'
		]);
		expect(new Set(Array.from(factory.factory.rules.keys()))).to.deep.equal(expectedRuleNames);
	});

	// endregion

	// region create

	it('can create known transaction with multiple overrides', () => {
		// Arrange:
		const typeRuleOverrides = new Map();
		typeRuleOverrides.set(Address, x => `${x} but amazing`);
		typeRuleOverrides.set(nc.Amount, () => 654321);
		typeRuleOverrides.set(PublicKey, x => `${x} PUBLICKEY`);
		const factory = testDescriptor.createFactory(typeRuleOverrides);

		// Act:
		const transaction = testDescriptor.createTransaction(factory)({
			type: 'namespace_registration_transaction',
			signerPublicKey: 'signerName',
			rentalFeeSink: 'fee sink',
			rentalFee: 'fake fee'
		});

		// Assert:
		expect(transaction.type).to.equal(nc.TransactionType.NAMESPACE_REGISTRATION);
		expect(transaction.version).to.equal(1);
		expect(transaction.network).to.deep.equal(nc.NetworkType.TESTNET);

		const stringToBytes = value => new TextEncoder().encode(value);
		expect(transaction.signerPublicKey).to.deep.equal(stringToBytes('signerName PUBLICKEY'));
		expect(transaction.rentalFeeSink).to.deep.equal(stringToBytes('fee sink but amazing'));
		expect(transaction.rentalFee).to.equal(654321);
	});

	// endregion

	// region address type conversion

	it('can create transaction with address', () => {
		// Arrange: this tests the custom type converter
		const factory = testDescriptor.createFactory();

		// Act:
		const transaction = testDescriptor.createTransaction(factory)({
			type: 'namespace_registration_transaction',
			signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
			rentalFeeSink: new Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB')
		});

		// Assert:
		expect(transaction.rentalFeeSink)
			.to.deep.equal(new nc.Address('4145424147424146415944515143494B424D474132445150434149524545595543554C424F474142'));
	});

	// endregion

	// region message encoding

	it('can create transfer with string message', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();

		// Act:
		const transaction = testDescriptor.createTransaction(factory)({
			type: 'transfer_transaction',
			signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
			message: {
				messageType: 'plain',
				message: 'You miss 100%% of the shots you don\'t take'
			}
		});

		// Assert:
		expect(transaction.message.message).to.deep.equal(new TextEncoder().encode('You miss 100%% of the shots you don\'t take'));
	});

	// endregion

	// region non verifiable

	const createTransferDescriptorWithSignature = signature => {
		const generateRandomValue = ModelType => (
			8 === ModelType.SIZE
				? BigInt(`0x${uint8ToHex(crypto.randomBytes(ModelType.SIZE))}`)
				: parseInt(`0x${uint8ToHex(crypto.randomBytes(ModelType.SIZE))})`, 16)
		);
		const generateRandomByteArray = ModelType => new ModelType(crypto.randomBytes(ModelType.SIZE));
		return {
			type: 'transfer_transaction_v1',
			timestamp: generateRandomValue(nc.Timestamp),
			signerPublicKey: generateRandomByteArray(PublicKey),
			signature,
			fee: generateRandomValue(nc.Amount),
			deadline: generateRandomValue(nc.Timestamp),
			recipientAddress: generateRandomByteArray(Address),
			amount: generateRandomValue(nc.Amount),
			message: {
				messageType: 'plain',
				message: 'Wayne Gretzky'
			}
		};
	};

	it('can convert verifiable transaction to non-verifiable', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();
		const signature = new Signature(crypto.randomBytes(Signature.SIZE));
		const transaction = testDescriptor.createTransaction(factory)(createTransferDescriptorWithSignature(signature));

		// Act:
		const nonVerifiableTransaction = TransactionFactory.toNonVerifiableTransaction(transaction);

		// Assert:
		expect(nonVerifiableTransaction.signature).to.equal(undefined);

		// - cut out size and signature from the buffer
		const verifiableBuffer = transaction.serialize();

		const offset = nc.TransactionType.TRANSFER.size + 1 + 2 + nc.NetworkType.TESTNET.size + nc.Timestamp.SIZE + 4 + nc.PublicKey.SIZE;
		const expectedNonVerifiableBuffer = new Uint8Array([
			...verifiableBuffer.subarray(0, offset),
			...verifiableBuffer.subarray(offset + 4 + nc.Signature.SIZE)
		]);
		expect(nonVerifiableTransaction.serialize()).to.deep.equal(expectedNonVerifiableBuffer);

		// - additionally check that serialized signature matches initial one
		expect(signature.bytes).to.deep.equal(verifiableBuffer.subarray(offset + 4, offset + 4 + nc.Signature.SIZE));
	});

	it('can convert non-verifiable transaction to non-verifiable', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();
		const signature = new Signature(crypto.randomBytes(Signature.SIZE));
		const transaction = testDescriptor.createTransaction(factory)(createTransferDescriptorWithSignature(signature));

		const nonVerifiableTransaction1 = TransactionFactory.toNonVerifiableTransaction(transaction);

		// Act:
		const nonVerifiableTransaction2 = TransactionFactory.toNonVerifiableTransaction(nonVerifiableTransaction1);

		// Assert:
		expect(nonVerifiableTransaction2.serialize()).to.deep.equal(nonVerifiableTransaction1.serialize());
	});

	// endregion
});
