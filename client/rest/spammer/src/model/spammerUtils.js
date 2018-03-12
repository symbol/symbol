const catapult = require('catapult-sdk');

module.exports = {
	signAndStitchAggregateTransaction: (codec, aggregateSignerKeyPair, keyPairs, transaction) => {
		// note: signed data does not contain cosignatures because transaction does not contain them
		const { transactionExtensions } = catapult.modelBinary;
		transactionExtensions.sign(codec, aggregateSignerKeyPair, transaction);
		const aggregateHash = transactionExtensions.hash(codec, transaction);

		if (keyPairs.find(keyPair => aggregateSignerKeyPair.publicKey === keyPair.publicKey))
			throw Error('aggregate signer key present in list of cosigners');

		transaction.cosignatures = keyPairs.map(keyPair => ({
			signer: keyPair.publicKey,
			signature: catapult.crypto.sign(keyPair, aggregateHash)
		}));
	}
};
