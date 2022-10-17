export default () => {
	const sampleAddress = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y';
	const sampleNamespaceId = 0xC01DFEE7FEEDDEADn;
	const sampleMosaicId = 0x7EDCBA90FEDCBA90n;

	return [
		{
			type: 'address_alias_transaction_v1',
			namespaceId: sampleNamespaceId,
			address: sampleAddress,
			aliasAction: 'link'
		},

		{
			type: 'mosaic_alias_transaction_v1',
			namespaceId: sampleNamespaceId,
			mosaicId: sampleMosaicId,
			aliasAction: 'link'
		}
	];
};
