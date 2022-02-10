from binascii import unhexlify

recipes = {
	'schema_name': 'TransferTransaction',
	'descriptors': [
		# comment: v2, plain, non-empty message, single mosaic
		{
			'type': 'transfer_transaction',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0xB,
			'message': {
				'message_type': 'plain',
				'message': b'hi!'
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				}
			]
		},
		# comment: v2, encrypted, non-empty message, single mosaic
		{
			'type': 'transfer_transaction',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'3F7FED92C755C444C492BD5DFD003A0F6F7E257A4DE7CC91E33EDD477F29C9BB'
					'5BD906BE5CC9E20B80A4F7C65C24727FAC4145AD42E055DBEDF516D96D386516'),
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				}
			]
		},
		# comment: v2, plain, non-empty message, multiple mosaics
		{
			'type': 'transfer_transaction',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0xB,
			'message': {
				'message_type': 'plain',
				'message': b'hi!'
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'almost'}, 'name': b'nonfungible'},
						'amount': 0x0000000000000002,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
						'amount': 0x000000000000007B,
					}
				}
			]
		},
		# comment: v2, encrypted, non-empty message, multiple mosaics
		{
			'type': 'transfer_transaction',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'48365F97EB76BAD136AC4630E792D1610B6591C9BF7DE0EA9D618FD7480AAE21'
					'A092BC8223F427B8263C7ED4EB665C945B569E42F48FF1A50A4CD20A3AF5173D'),
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'almost'}, 'name': b'nonfungible'},
						'amount': 0x0000000000000002,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
						'amount': 0x000000000000007B,
					}
				}
			]
		}
	]
}
