export default () => {
	const sampleAddress = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y';
	const sampleMosaicId = 0x7EDCBA90FEDCBA90n;

	return [
		// mosaics but no message
		{
			type: 'transfer_transaction_v1',
			recipientAddress: sampleAddress,
			mosaics: [
				{ mosaicId: sampleMosaicId, amount: 12345_000000n }
			]
		},

		// message but no mosaics
		{
			type: 'transfer_transaction_v1',
			recipientAddress: sampleAddress,
			message: 'Wayne Gretzky'
		},

		// mosaics and message
		{
			type: 'transfer_transaction_v1',
			recipientAddress: sampleAddress,
			mosaics: [
				{ mosaicId: sampleMosaicId, amount: 12345_000000n }
			],
			message: 'You miss 100%% of the shots you don’t take'
		}
	];
};
