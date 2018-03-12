const { expect } = require('chai');
const catapult = require('catapult-sdk');
const spammerUtils = require('../../src/model/spammerUtils');
const transactionFactory = require('../../src/model/transactionFactory');
const test = require('../testUtils');

const createKey = catapult.crypto.createKeyPairFromPrivateKeyString;
const modelCodec = catapult.plugins.catapultModelSystem.configure(['transfer', 'aggregate'], {}).codec;

describe('spammer utils', () => {
	describe('sign and stitch aggregate transaction', () => {
		const createTransaction = signer =>
			transactionFactory.createAggregateTransaction({ signerPublicKey: signer.publicKey, networkId: 0xA5 }, []);

		const createRandomKey = () => createKey(catapult.utils.convert.uint8ToHex(test.random.bytes(catapult.constants.sizes.signer)));

		it('adds expected number of cosignatures', () => {
			// Arrange:
			const signer = createRandomKey();
			const cosigners = [createRandomKey(), createRandomKey()];
			const transaction = createTransaction(signer);

			// Sanity:
			expect(transaction.cosignatures).to.equal(undefined);

			// Act:
			spammerUtils.signAndStitchAggregateTransaction(modelCodec, signer, cosigners, transaction);

			// Assert:
			expect(transaction.cosignatures.length).to.equal(cosigners.length);
			cosigners.forEach((keyPair, i) => {
				expect(keyPair.publicKey).to.deep.equal(transaction.cosignatures[i].signer);
			});
		});

		it('throws if signer key is in cosigners', () => {
			// Arrange:
			const signer = createRandomKey();
			const cosigners = [createRandomKey(), signer, createRandomKey()];
			const transaction = createTransaction(signer);

			// Act:
			expect(() => spammerUtils.signAndStitchAggregateTransaction(modelCodec, signer, cosigners, transaction))
				.to.throw('aggregate signer key present in list of cosigners');
		});
	});
});
