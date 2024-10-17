SCHEMA_NAME = 'MosaicAliasTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_alias_transaction_v1',
			'namespace_id': 0xB6F1FD51147987A4,
			'mosaic_id': 0x000000000000000A,
			'alias_action': 'link'
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_alias_transaction_v1',
			'namespace_id': 0xE1499A8D01FCD82A,
			'mosaic_id': 0xCAF5DD1286D7CC4C,
			'alias_action': 'unlink'
		}
	}
]
