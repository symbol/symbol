export default () => {
	const sampleAddress = 'TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C';
	const textEncoder = new TextEncoder();

	return [
		// mosaics but no message
		{
			type: 'transfer_transaction_v2',
			recipientAddress: sampleAddress,
			amount: 12345_000000n
		},

		// message but no mosaics
		{
			type: 'transfer_transaction_v2',
			recipientAddress: sampleAddress,
			message: {
				messageType: 'plain',
				message: 'You miss 100%% of the shots you don’t take'
			}
		},

		// mosaics and message
		{
			type: 'transfer_transaction_v2',
			recipientAddress: sampleAddress,
			amount: 12345_000000n,
			message: {
				messageType: 'plain',
				message: ' Wayne Gretzky'
			}
		},

		// mosaic bags
		{
			type: 'transfer_transaction_v2',
			recipientAddress: sampleAddress,
			amount: 1_000000n,
			message: {
				messageType: 'plain',
				message: ' Wayne Gretzky'
			},
			mosaics: [
				{
					mosaic: {
						mosaicId: { namespaceId: { name: textEncoder.encode('nem') }, name: textEncoder.encode('xem') },
						amount: 12345_000000n
					}
				},
				{
					mosaic: {
						mosaicId: {
							namespaceId: { name: textEncoder.encode('magic') },
							name: textEncoder.encode('some_mosaic_with_divisibility_2')
						},
						amount: 5_00n
					}
				}
			]
		},

		// mosaics and message V1
		{
			type: 'transfer_transaction_v1',
			recipientAddress: sampleAddress,
			amount: 12345_000000n,
			message: {
				messageType: 'plain',
				message: ' Wayne Gretzky'
			}
		}
	];
};
