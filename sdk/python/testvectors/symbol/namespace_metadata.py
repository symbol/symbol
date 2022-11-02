SCHEMA_NAME = 'NamespaceMetadataTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'namespace_metadata_transaction_v1',
			'target_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
			'scoped_metadata_key': 0xA,
			'target_namespace_id': 0x00000000000003E8,
			'value_size_delta': 0xA,
			'value': 'ABC123'
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'namespace_metadata_transaction_v1',
			'target_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
			'scoped_metadata_key': 0xA,
			'target_namespace_id': 0x00000000000003E8,
			'value_size_delta': -3,
			'value': 'ABC123'
		}
	}
]
