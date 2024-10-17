SCHEMA_NAME = 'MosaicAddressRestrictionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_address_restriction_transaction_v1',
			'mosaic_id': 0x0000000000000001,
			'restriction_key': 0x1234567890ABCAEF,
			'previous_restriction_value': 0x9,
			'new_restriction_value': 0x8,
			'target_address': 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'
		}
	}
]
