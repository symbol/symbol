export default () => {
	const sampleAddress = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y';
	const sampleMosaicId = 0x7EDCBA90FEDCBA90n;

	return [
		{
			type: 'mosaic_global_restriction_transaction_v1',
			mosaicId: sampleMosaicId,
			referenceMosaicId: 0n,
			restrictionKey: 0x0A0D474E5089n,
			previousRestrictionValue: 0n,
			newRestrictionValue: 2n,
			previousRestrictionType: 0,
			newRestrictionType: 'ge'
		},

		{
			type: 'mosaic_address_restriction_transaction_v1',
			mosaicId: sampleMosaicId,
			restrictionKey: 0x0A0D474E5089n,
			previousRestrictionValue: 0n,
			newRestrictionValue: 5n,
			targetAddress: sampleAddress
		}
	];
};
