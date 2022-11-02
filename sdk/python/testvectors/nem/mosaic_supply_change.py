SCHEMA_NAME = 'MosaicSupplyChangeTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	# comment: increase
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': {
				'namespace_id': {'name': b'banksters'},
				'name': b'bailout'
			},
			'action': 'increase',
			'delta': 0x000000000005464E
		}
	},
	# comment: decrease
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': {
				'namespace_id': {'name': b'banksters'},
				'name': b'bailout',
			},
			'action': 'decrease',
			'delta': 0x00000000001DCD65,
		}
	}
]
