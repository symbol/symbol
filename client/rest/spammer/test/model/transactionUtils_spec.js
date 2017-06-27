import { expect } from 'chai';
import catapult from 'catapult-sdk';
import transactionUtils from '../../src/model/transactionUtils';
import networkTime from '../../src/utils/networkTime';
import test from '../testUtils';

const Signature_Size = catapult.constants.sizes.signature;
const Address_Decoded_Size = catapult.constants.sizes.addressDecoded;
const createKey = catapult.crypto.createKeyPairFromPrivateKeyString;
const modelCodec = catapult.plugins.catapultModelSystem.configure(['transfer']).codec;

describe('transaction utils', () => {
	const Private_Key = '8D31B712AB28D49591EAF5066E9E967B44507FC19C3D54D742F7B3A255CFF4AB';
	const Mijin_Test_Network = catapult.model.networkInfo.networks.mijinTest.id;

	function createTransaction(keyPair) {
		return transactionUtils.createRandomTransfer(
			{ signerPublicKey: keyPair.publicKey, networkId: Mijin_Test_Network, transferId: 0x1234 },
			() => test.random.bytes(Address_Decoded_Size));
	}

	describe('random transfer', () => {
		it('has reasonable data', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const recipient = test.random.bytes(Address_Decoded_Size);

			// Act:
			const transaction = transactionUtils.createRandomTransfer(
				{ signerPublicKey: keyPair.publicKey, networkId: Mijin_Test_Network, transferId: 0x1234 },
				() => recipient);

			// Assert:
			const txDeadline = catapult.utils.uint64.compact(transaction.deadline);
			expect(transaction.signer).to.deep.equal(keyPair.publicKey);
			expect(transaction.signature).to.deep.equal(new Uint8Array(Signature_Size));
			expect(transaction.version).to.equal(36867);
			expect(transaction.type).to.equal(0x4101);
			expect(transaction.fee).to.deep.equal([0x00, 0x00]);
			expect(txDeadline).to.be.at.least(networkTime.getNetworkTime());
			expect(txDeadline).to.be.at.most(networkTime.getNetworkTime() + (24 * 60 * 60 * 1000));
			expect(transaction.recipient).to.deep.equal(recipient);
			expect(transaction.message.type).to.equal(0);
			expect(transaction.message.payload).to.deep.equal(Uint8Array.of([0x34, 0x12, 0x00, 0x00]));
			expect(transaction.mosaics[0].id).to.deep.equal([0xD95FCF29, 0xD525AD41]);
		});
	});

	describe('validate', () => {
		it('success for signed transaction', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const transaction = createTransaction(keyPair);
			transactionUtils.signTransaction(modelCodec, keyPair, transaction);

			// Act:
			const isValid = transactionUtils.verifyTransactionSignature(modelCodec, transaction);

			// Assert:
			expect(isValid).to.equal(true);
		});

		it('failure for unsigned transaction', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const transaction = createTransaction(keyPair);

			// Act:
			const isValid = transactionUtils.verifyTransactionSignature(modelCodec, transaction);

			// Assert:
			expect(isValid).to.equal(false);
		});

		it('failure for altered signed transaction', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const transaction = createTransaction(keyPair);
			transactionUtils.signTransaction(modelCodec, keyPair, transaction);
			transaction.fee[0] += 1;

			// Act:
			const isValid = transactionUtils.verifyTransactionSignature(modelCodec, transaction);

			// Assert:
			expect(isValid).to.equal(false);
		});

		it('failure for signed transaction with altered signature', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const transaction = createTransaction(keyPair);
			transactionUtils.signTransaction(modelCodec, keyPair, transaction);
			transaction.signature[0] ^= 0xFF;

			// Act:
			const isValid = transactionUtils.verifyTransactionSignature(modelCodec, transaction);

			// Assert:
			expect(isValid).to.equal(false);
		});
	});
});
