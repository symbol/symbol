transactions = [  # pylint: disable=duplicate-code
	# bonded aggregate v1, transfer + supply change
	{
		'schema_name': 'AggregateBondedTransactionV1',
		'descriptor': {
			'aggregate': {'type': 'aggregate_bonded_transaction_v1'},
			'embedded': [
				{
					'type': 'transfer_transaction_v1',
					'recipient_address': 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ',
					'mosaics': [
						{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000065}
					],
				},
				{
					'type': 'mosaic_supply_change_transaction_v1',
					'mosaic_id': 0x57701A9B6E746989,
					'action': 'increase',
					'delta': 0xA
				}
			],
			'num_cosignatures': 2
		}
	},

	# bonded aggregate v2, transfer + supply change
	{
		'schema_name': 'AggregateBondedTransactionV2',
		'descriptor': {
			'aggregate': {'type': 'aggregate_bonded_transaction_v2'},
			'embedded': [
				{
					'type': 'transfer_transaction_v1',
					'recipient_address': 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ',
					'mosaics': [
						{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
					],
				},
				{
					'type': 'mosaic_supply_change_transaction_v1',
					'mosaic_id': 0x57701A9B6E746988,
					'action': 'increase',
					'delta': 0xA
				}
			],
			'num_cosignatures': 2
		}
	}
]
