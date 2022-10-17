recipes = {
	'schema_name': 'MosaicSupplyChangeTransactionV1',
	'descriptors': [
		# comment: increase
		{
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': {
				'namespace_id': {'name': b'banksters'},
				'name': b'bailout'
			},
			'action': 'increase',
			'delta': 0x000000000005464E
		},
		# comment: decrease
		{
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': {
				'namespace_id': {'name': b'banksters'},
				'name': b'bailout',
			},
			'action': 'decrease',
			'delta': 0x00000000001DCD65,
		}
	]
}
