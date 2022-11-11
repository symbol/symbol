SCHEMA_NAME = 'MultisigTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	# comment: two transfers
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'aggregate': {'type': 'multisig_transaction_v1'},
			# comment: v2, encrypted, non-empty message, multiple mosaics
			'embedded': {
				'type': 'transfer_transaction_v2',
				'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
				'amount': 0x00000000004C4B41,
				'message': {
					'message_type': 'plain',
					'message': 'Good Morning',
				},
				'mosaics': [
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
							'amount': 0x000000000015464E,
						}
					},
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
							'amount': 0x000000000000017B,
						}
					}
				]
			},
			'num_cosignatures': 3
		}
	}
]
