export default () => {
	const sampleAddress = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y';
	const sampleMosaicId = 0x7EDCBA90FEDCBA90n;

	return [
		// allow incoming transactions only from address below
		{
			type: 'account_address_restriction_transaction_v1',
			restrictionFlags: 'address',
			restrictionAdditions: [sampleAddress]
		},

		// block transactions outgoing to given address
		// note: block and allow restrictions are mutually exclusive, documentation
		// https://docs.symbol.dev/concepts/account-restriction.html#account-restriction
		{
			type: 'account_address_restriction_transaction_v1',
			restrictionFlags: 'address outgoing block',
			restrictionAdditions: [sampleAddress]
		},

		{
			type: 'account_mosaic_restriction_transaction_v1',
			restrictionFlags: 'mosaic_id',
			restrictionAdditions: [sampleMosaicId]
		},

		// allow only specific transaction types
		{
			type: 'account_operation_restriction_transaction_v1',
			restrictionFlags: 'outgoing',
			restrictionAdditions: [
				'transfer',
				'account_key_link',
				'vrf_key_link',
				'voting_key_link',
				'node_key_link'
			]
		}
	];
};
