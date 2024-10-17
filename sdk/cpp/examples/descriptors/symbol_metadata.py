def descriptor_factory():
	sample_address = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y'
	sample_namespace_id = 0xC01DFEE7FEEDDEAD
	sample_mosaic_id = 0x7EDCBA90FEDCBA90
	value1 = 'much coffe, such wow'
	value2 = 'Once upon a midnight dreary'
	value3 = 'while I pondered, weak and weary'

	templates = [
		{
			'type': 'account_metadata_transaction_v1',
			'target_address': sample_address,
			'scoped_metadata_key': 0xC0FFE
		},

		{
			'type': 'mosaic_metadata_transaction_v1',
			'target_mosaic_id': sample_mosaic_id,
			'target_address': sample_address,
			'scoped_metadata_key': 0xFACADE
		},

		{
			'type': 'namespace_metadata_transaction_v1',
			'target_namespace_id': sample_namespace_id,
			'target_address': sample_address,
			'scoped_metadata_key': 0xC1CADA
		}
	]

	return [
		{**templates[0], 'value_size_delta': len(value1), 'value': value1},
		{**templates[0], 'value_size_delta': -5, 'value': value1[:-5]},

		{**templates[1], 'value_size_delta': len(value2), 'value': value2},
		{**templates[1], 'value_size_delta': -5, 'value': value2[:-5]},

		{**templates[2], 'value_size_delta': len(value3), 'value': value3},
		{**templates[2], 'value_size_delta': -5, 'value': value3[:-5]}
	]
