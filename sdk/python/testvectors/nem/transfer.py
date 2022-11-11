from binascii import unhexlify

transactions = [  # pylint: disable=duplicate-code
	# comment: v1, no-message
	{
		'schema_name': 'TransferTransactionV1',
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
		}
	},
	# comment: v1, plain empty message
	{
		'schema_name': 'TransferTransactionV1',
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message': {
				'message_type': 'plain',
				'message': '',
			}
		}
	},
	# comment: v1, plain non-empty message
	{
		'schema_name': 'TransferTransactionV1',
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message': {
				'message_type': 'plain',
				'message': b'hi!'
			}
		}
	},
	# comment: v1, encrypted, non-empty message
	{
		'schema_name': 'TransferTransactionV1',
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'642E756F0DBF4BE90EFCAB7AB42F297ED64EDC83E05062F8E49F6CAC61FBBA9F'
					'8D198B4B79A33096C58977C9134FAEE74307A6E55156BF6C2718E2B0D44BE050'),
			}
		}
	},

	# comment: v2, plain, non-empty message, single mosaic
	{
		'schema_name': 'TransferTransactionV2',
		'descriptor': {
			'type': 'transfer_transaction_v2',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
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
		}
	},
	# comment: v2, encrypted, non-empty message, single mosaic
	{
		'schema_name': 'TransferTransactionV2',
		'descriptor': {
			'type': 'transfer_transaction_v2',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
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
		}
	},
	# comment: v2, plain, non-empty message, multiple mosaics
	{
		'schema_name': 'TransferTransactionV2',
		'descriptor': {
			'type': 'transfer_transaction_v2',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
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
		}
	},
	# comment: v2, encrypted, non-empty message, multiple mosaics
	{
		'schema_name': 'TransferTransactionV2',
		'descriptor': {
			'type': 'transfer_transaction_v2',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
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
	}
]
