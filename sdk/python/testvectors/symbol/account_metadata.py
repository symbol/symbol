SAMPLE_ADDRESS = 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'
SCHEMA_NAME = 'AccountMetadataTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_metadata_transaction_v1',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'value_size_delta': 0xA,
			'value': b'123BAC'
		},
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_metadata_transaction_v1',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xABCAEF,
			'value_size_delta': -6,
			'value': b'123BAC'
		}
	}
]
