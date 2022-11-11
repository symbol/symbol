SCHEMA_NAME = 'MosaicSupplyChangeTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': 0x57701A9B6E746988,
			'action': 'increase',
			'delta': 0xA
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': 0xCAF5DD1286D7CC4C,
			'action': 'decrease',
			'delta': 0xA
		}
	}
]
