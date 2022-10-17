import symbolSdk from '../../src/index.js';

export default () => {
	const sampleAddress = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y';
	const sampleMosaicId = 0x7EDCBA90FEDCBA90n;
	const secret = 'C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000';

	return [
		// note: only network currency can be used as a mosaic in hash lock
		{
			type: 'hash_lock_transaction_v1',
			mosaic: { mosaicId: sampleMosaicId, amount: 123_000000n },
			duration: 123n,
			hash: symbolSdk.Hash256.zero()
		},

		{
			type: 'secret_lock_transaction_v1',
			mosaic: { mosaicId: sampleMosaicId, amount: 123_000000n },
			duration: 123n,
			recipientAddress: sampleAddress,
			secret,
			hashAlgorithm: 'hash_160'
		},

		{
			type: 'secret_proof_transaction_v1',
			recipientAddress: sampleAddress,
			secret,
			hashAlgorithm: 'hash_160',
			proof: (new symbolSdk.ByteArray(4, 'C1ECFDFC')).bytes
		}
	];
};
