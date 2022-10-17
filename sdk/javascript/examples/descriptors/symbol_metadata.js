export default () => {
	const sampleAddress = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y';
	const sampleNamespaceId = 0xC01DFEE7FEEDDEADn;
	const sampleMosaicId = 0x7EDCBA90FEDCBA90n;
	const value1 = 'much coffe, such wow';
	const value2 = 'Once upon a midnight dreary';
	const value3 = 'while I pondered, weak and weary';

	const templates = [
		{
			type: 'account_metadata_transaction_v1',
			targetAddress: sampleAddress,
			scopedMetadataKey: 0xC0FFEn
		},

		{
			type: 'mosaic_metadata_transaction_v1',
			targetMosaicId: sampleMosaicId,
			targetAddress: sampleAddress,
			scopedMetadataKey: 0xFACADEn
		},

		{
			type: 'namespace_metadata_transaction_v1',
			targetNamespaceId: sampleNamespaceId,
			targetAddress: sampleAddress,
			scopedMetadataKey: 0xC1CADAn
		}
	];

	return [
		{ ...templates[0], valueSizeDelta: value1.length, value: value1 },
		{ ...templates[0], valueSizeDelta: -5, value: value1.substring(0, value1.length - 5) },

		{ ...templates[1], valueSizeDelta: value2.length, value: value2 },
		{ ...templates[1], valueSizeDelta: -5, value: value2.substring(0, value2.length - 5) },

		{ ...templates[2], valueSizeDelta: value3.length, value: value3 },
		{ ...templates[2], valueSizeDelta: -5, value: value3.substring(0, value3.length - 5) }
	];
};
