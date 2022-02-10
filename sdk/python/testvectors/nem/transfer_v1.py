from binascii import unhexlify

recipes = {
	'schema_name': 'TransferTransactionV1',
	'descriptors': [
		# comment: v1, no-message
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x0
		},
		# comment: v1, plain empty message
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x8,
			'message': {
				'message_type': 'plain',
				'message': '',
			}
		},
		# comment: v1, plain non-empty message
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0xB,
			'message': {
				'message_type': 'plain',
				'message': b'hi!'
			}
		},
		# comment: v1, encrypted, non-empty message
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'642E756F0DBF4BE90EFCAB7AB42F297ED64EDC83E05062F8E49F6CAC61FBBA9F'
					'8D198B4B79A33096C58977C9134FAEE74307A6E55156BF6C2718E2B0D44BE050'),
			}
		}
	]
}
