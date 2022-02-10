aggregate_recipes = {
	# comment: two transfers
	'schema_name': 'MultisigTransaction',
	'descriptors': [
		{
			'aggregate': {'type': 'multisig_transaction'},
			# comment: v2, encrypted, non-empty message, multiple mosaics
			'embedded': {
				'type': 'transfer_transaction',
				'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
				'amount': 0x00000000004C4B40,
				'message_envelope_size': 0x14,
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
	]
}
