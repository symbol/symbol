SCHEMA_NAME = 'MosaicDefinitionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'duration': 0x0000000000002710,
			'nonce': 0x00000000,
			'flags': 'restrictable supply_mutable',
			'divisibility': 4
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'duration': 0x00000000000003E8,
			'nonce': 0xB884DEE6,
			'flags': 'none',
			'divisibility': 3
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'duration': 0x0000000000000000,
			'nonce': 0xB884DEE6,
			'flags': 'revokable transferable',
			'divisibility': 2
		}
	}
]
