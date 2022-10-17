def descriptor_factory():
	sample_address = 'TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C'

	return [
		# mosaics but no message
		{
			'type': 'transfer_transaction_v2',
			'recipient_address': sample_address,
			'amount': 12345_000000
		},

		# message but no mosaics
		{
			'type': 'transfer_transaction_v2',
			'recipient_address': sample_address,
			'message': {
				'message_type': 'plain',
				'message': 'You miss 100%% of the shots you don’t take'
			}
		},

		# mosaics and message
		{
			'type': 'transfer_transaction_v2',
			'recipient_address': sample_address,
			'amount': 12345_000000,
			'message': {
				'message_type': 'plain',
				'message': ' Wayne Gretzky'
			}
		},

		# mosaic bags
		{
			'type': 'transfer_transaction_v2',
			'recipient_address': sample_address,
			'amount': 1_000000,
			'message': {
				'message_type': 'plain',
				'message': ' Wayne Gretzky'
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': 'nem'.encode('utf8')}, 'name': 'xem'.encode('utf8')},
						'amount': 12345_000000
					}
				},
				{
					'mosaic': {
						'mosaic_id': {
							'namespace_id': {'name': 'magic'.encode('utf8')},
							'name': 'some_mosaic_with_divisibility_2'.encode('utf8')
						},
						'amount': 5_00
					}
				}
			]
		},

		# mosaics and message V1
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': sample_address,
			'amount': 12345_000000,
			'message': {
				'message_type': 'plain',
				'message': ' Wayne Gretzky'
			}
		}
	]
