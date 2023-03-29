transactions = [  # pylint: disable=duplicate-code
	# comment: two transfers, V1
	{
		'schema_name': 'MultisigTransactionV1',
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
	},
	# comment: two transfers, V2
	{
		'schema_name': 'MultisigTransactionV2',
		'descriptor': {
			'aggregate': {'type': 'multisig_transaction_v2'},
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
