import { PublicKey, Signature } from '../../src/CryptoTypes.js';
import { Address, Network } from '../../src/nem/Network.js';
import TransactionFactory from '../../src/nem/TransactionFactory.js';
import * as nc from '../../src/nem/models.js';
import { hexToUint8, uint8ToHex } from '../../src/utils/converter.js';
import { runBasicTransactionFactoryTests } from '../test/basicTransactionFactoryTests.js';
import { expect } from 'chai';
import crypto from 'crypto';

describe('transaction factory (NEM)', () => {
	const TEST_SIGNER_PUBLIC_KEY = new PublicKey(crypto.randomBytes(PublicKey.SIZE));

	const assertTransfer = transaction => {
		expect(transaction.type).to.equal(nc.TransactionType.TRANSFER);
		expect(transaction.version).to.equal(2);
		expect(transaction.network).to.deep.equal(nc.NetworkType.TESTNET);
	};

	const testDescriptor = {
		name: 'Transaction',
		transactionTypeName: 'transfer_transaction_v2',
		createFactory: typeRuleOverrides => new TransactionFactory(Network.TESTNET, typeRuleOverrides),
		createTransaction: factory => ((descriptor, autosort = true) => factory.create(descriptor, autosort)),
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
		const ruleNames = new Set(factory.ruleNames);
		expect(ruleNames).to.deep.equal(expectedRuleNames);
	});

	// endregion

	// region lookupTransactionName

	describe('lookupTransactionName', () => {
		it('can lookup known transaction', () => {
			expect(TransactionFactory.lookupTransactionName(nc.TransactionType.TRANSFER, 1)).to.equal('transfer_transaction_v1');
			expect(TransactionFactory.lookupTransactionName(nc.TransactionType.TRANSFER, 2)).to.equal('transfer_transaction_v2');
			expect(TransactionFactory.lookupTransactionName(nc.TransactionType.MULTISIG, 1)).to.equal('multisig_transaction_v1');
		});

		it('cannot lookup unknown transaction', () => {
			expect(() => TransactionFactory.lookupTransactionName(new nc.TransactionType(123), 1)).to.throw('invalid enum value 123');
		});
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
			type: 'namespace_registration_transaction_v1',
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
			type: 'namespace_registration_transaction_v1',
			signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
			rentalFeeSink: new Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB')
		});

		// Assert:
		const expectedAddressBytes = hexToUint8('4145424147424146415944515143494B424D474132445150434149524545595543554C424F474142');
		expect(transaction.rentalFeeSink).to.deep.equal(new nc.Address(expectedAddressBytes));
	});

	// endregion

	// region sorting

	const createUnorderedDescriptor = () => ({
		type: 'multisig_account_modification_transaction_v2',
		signerPublicKey: TEST_SIGNER_PUBLIC_KEY,
		modifications: [
			{
				modification: {
					modificationType: 'delete_cosignatory',
					cosignatoryPublicKey: new PublicKey('D79936328C188A4416224ABABF580CA2C5C8D852248DB1933FE4BC0DCA0EE7BC')
				}
			},
			{
				modification: {
					modificationType: 'add_cosignatory',
					cosignatoryPublicKey: new PublicKey('5D378657691CAD70CE35A46FB88CB134232B0B6B3655449C019A1F5F20AE9AAD')
				}
			}
		]
	});

	it('can create transaction with out of order array when autosort is enabled', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();

		// Act:
		const transaction = testDescriptor.createTransaction(factory)(createUnorderedDescriptor());

		// Assert: modifications were reordered
		expect(transaction.modifications[0].modification.modificationType)
			.to.deep.equal(nc.MultisigAccountModificationType.ADD_COSIGNATORY);
		expect(transaction.modifications[1].modification.modificationType)
			.to.deep.equal(nc.MultisigAccountModificationType.DELETE_COSIGNATORY);
	});

	it('cannot create transaction with out of order array when autosort is disabled', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();

		// Act:
		const transaction = testDescriptor.createTransaction(factory)(createUnorderedDescriptor(), false);

		// Assert: modifications were NOT reordered (serialization will fail)
		expect(transaction.modifications[0].modification.modificationType)
			.to.deep.equal(nc.MultisigAccountModificationType.DELETE_COSIGNATORY);
		expect(transaction.modifications[1].modification.modificationType)
			.to.deep.equal(nc.MultisigAccountModificationType.ADD_COSIGNATORY);

		expect(() => transaction.serialize()).to.throw(RangeError);
	});

	// endregion

	// region message encoding

	it('can create transfer with string message', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();

		// Act:
		const transaction = testDescriptor.createTransaction(factory)({
			type: 'transfer_transaction_v2',
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

		// Assert: nonVerifiableTransaction does not contain signature but source transaction does
		expect(Object.prototype.hasOwnProperty.call(transaction, '_signature')).to.equal(true);
		expect(Object.prototype.hasOwnProperty.call(nonVerifiableTransaction, '_signature')).to.equal(false);

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
