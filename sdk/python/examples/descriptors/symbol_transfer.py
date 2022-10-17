def descriptor_factory():
	sample_address = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y'
	sample_mosaic_id = 0x7EDCBA90FEDCBA90

	return [
		# mosaics but no message
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': sample_address,
			'mosaics': [
				{'mosaic_id': sample_mosaic_id, 'amount': 12345_000000}
			]
		},

		# message but no mosaics
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': sample_address,
			'message': 'Wayne Gretzky'
		},

		# mosaics and message
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': sample_address,
			'mosaics': [
				{'mosaic_id': sample_mosaic_id, 'amount': 12345_000000}
			],
			'message': 'You miss 100%% of the shots you don’t take'
		}
	]
