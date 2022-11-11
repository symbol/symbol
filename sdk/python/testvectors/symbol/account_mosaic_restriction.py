SCHEMA_NAME = 'AccountMosaicRestrictionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_mosaic_restriction_transaction_v1',
			'restriction_flags': 'mosaic_id',
			'restriction_additions': [0x00000000000003E8],
			'restriction_deletions': [0x00000000000007D0]
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_mosaic_restriction_transaction_v1',
			'restriction_flags': 'mosaic_id block',
			'restriction_additions': [0xCAF5DD1286D7CC4C],
			'restriction_deletions': []
		}
	}
]
