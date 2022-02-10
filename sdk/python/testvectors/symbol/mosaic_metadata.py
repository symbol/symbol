SAMPLE_ADDRESS = 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'


recipes = {
	'schema_name': 'MosaicMetadataTransaction',
	'descriptors': [
		{
			'type': 'mosaic_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'target_mosaic_id': 0x00000000000003E8,
			'value_size_delta': 0xA,
			'value': b'123ABC'
		},
		{
			'type': 'mosaic_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'target_mosaic_id': 0x00000000000003E8,
			'value_size_delta': -5,
			'value': b'123ABC'
		}
	]
}
