import symbolSdk from '../../src/index.js';

const { nem } = symbolSdk;

export default () => {
	const sampleAddress = 'TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C';
	const textEncoder = new TextEncoder();

	// HACK: until fixed
	const levy = new nem.MosaicLevy();
	levy.transferFeeType = nem.MosaicTransferFeeType.ABSOLUTE;
	levy.recipientAddress.bytes = (new nem.Address(sampleAddress)).bytes;
	levy.mosaicId = new nem.MosaicId();
	levy.mosaicId.namespaceId = new nem.NamespaceId();
	levy.mosaicId.namespaceId.name = textEncoder.encode('lieutenant');
	levy.mosaicId.name = textEncoder.encode('colonel');
	levy.fee = new nem.Amount(632_0000n);

	return [
		// without properties
		{
			type: 'mosaic_definition_transaction_v1',
			rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
			rentalFee: 50000n * 1000000n,

			mosaicDefinition: {
				ownerPublicKey: '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				id: { namespaceId: { name: textEncoder.encode('genes') }, name: textEncoder.encode('memes') },
				description: textEncoder.encode('Not really valuable mosaic'),
				properties: [
				],
				levy: {}
			}
		},

		// with properties
		{
			type: 'mosaic_definition_transaction_v1',
			rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
			rentalFee: 50000n * 1000000n,

			mosaicDefinition: {
				ownerPublicKey: '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				id: { namespaceId: { name: textEncoder.encode('genes') }, name: textEncoder.encode('memes') },
				description: textEncoder.encode('Not really valuable mosaic'),
				properties: [
					{ property: { name: textEncoder.encode('divisibility'), value: textEncoder.encode('3') } },
					{ property: { name: textEncoder.encode('initialSupply'), value: textEncoder.encode('123_000') } },
					{ property: { name: textEncoder.encode('supplyMutable'), value: textEncoder.encode('false') } },
					{ property: { name: textEncoder.encode('transferable'), value: textEncoder.encode('true') } }
				],
				levy: {}
			}
		},

		// with levy
		{
			type: 'mosaic_definition_transaction_v1',
			rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
			rentalFee: 50000n * 1000000n,

			mosaicDefinition: {
				ownerPublicKey: '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				id: { namespaceId: { name: textEncoder.encode('genes') }, name: textEncoder.encode('memes') },
				description: textEncoder.encode('Not really valuable mosaic'),
				properties: [
				],
				levy: {
					transferFeeType: 'absolute',
					recipientAddress: sampleAddress,
					mosaicId: { namespaceId: { name: textEncoder.encode('lieutenant') }, name: textEncoder.encode('colonel') },
					fee: 632_0000n
				}
			}
		},

		// supply change
		{
			type: 'mosaic_supply_change_transaction_v1',
			mosaicId: { namespaceId: { name: textEncoder.encode('genes') }, name: textEncoder.encode('memes') },
			action: 'increase',
			delta: 321_000n
		}
	];
};
