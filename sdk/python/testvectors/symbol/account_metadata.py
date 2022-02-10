SAMPLE_ADDRESS = 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'


recipes = {
	'schema_name': 'AccountMetadataTransaction',
	'descriptors': [
		{
			'type': 'account_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'value_size_delta': 0xA,
			'value': b'123BAC'
		},
		{
			'type': 'account_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xABCAEF,
			'value_size_delta': -6,
			'value': b'123BAC'
		}
	]
}
