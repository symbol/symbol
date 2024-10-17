SCHEMA_NAME = 'MosaicGlobalRestrictionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_global_restriction_transaction_v1',
			'mosaic_id': 0x1A05987643477C07,
			'reference_mosaic_id': 0x6AF25E2B25258026,
			'restriction_key': 0x1,
			'previous_restriction_value': 0x9,
			'new_restriction_value': 0x8,
			'previous_restriction_type': 'eq',
			'new_restriction_type': 'ge'
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_global_restriction_transaction_v1',
			'mosaic_id': 0x5623C1654EEE3F51,
			'reference_mosaic_id': 0x0000000000000000,
			'restriction_key': 0x115C,
			'previous_restriction_value': 0x0,
			'new_restriction_value': 0x0,
			'previous_restriction_type': 'none',
			'new_restriction_type': 'ge'
		}
	}
]
