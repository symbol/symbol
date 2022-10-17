def descriptor_factory():
	sample_address = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y'
	sample_mosaic_id = 0x7EDCBA90FEDCBA90

	return [
		# allow incoming transactions only from address below
		{
			'type': 'account_address_restriction_transaction_v1',
			'restriction_flags': 'address',
			'restriction_additions': [sample_address]
		},

		# block transactions outgoing to given address
		# note: block and allow restrictions are mutually exclusive, documentation
		# https://docs.symbol.dev/concepts/account-restriction.html#account-restriction
		{
			'type': 'account_address_restriction_transaction_v1',
			'restriction_flags': 'address outgoing block',
			'restriction_additions': [sample_address]
		},

		{
			'type': 'account_mosaic_restriction_transaction_v1',
			'restriction_flags': 'mosaic_id',
			'restriction_additions': [sample_mosaic_id]
		},

		# allow only specific transaction types
		{
			'type': 'account_operation_restriction_transaction_v1',
			'restriction_flags': 'outgoing',
			'restriction_additions': [
				'transfer',
				'account_key_link',
				'vrf_key_link',
				'voting_key_link',
				'node_key_link'
			]
		}
	]
