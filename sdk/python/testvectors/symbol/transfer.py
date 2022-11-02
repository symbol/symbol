from binascii import unhexlify

SAMPLE_ADDRESS_ALIAS = 'TFIXOYLI2JBFPWAAAAAAAAAAAAAAAAAAAAAAAAA'
SAMPLE_ADDRESS = 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I'
SCHEMA_NAME = 'TransferTransactionV1'


transactions = [  # pylint: disable=duplicate-code
	# comment: no message, 1 mosaic
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			]
		}
	},
	# comment: no message, 2 mosaics
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x0000000000000064, 'amount': 0x0000000000000002},
				{'mosaic_id': 0x00000000000000C8, 'amount': 0x0000000000000001}
			]
		}
	},
	# comment: no message, 3 mosaics
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x67F2B76F28BD36BA, 'amount': 0x0000000000000003},
				{'mosaic_id': 0x77A1969932D987D7, 'amount': 0x0000000000000002},
				{'mosaic_id': 0xD525AD41D95FCF29, 'amount': 0x0000000000000001}
			]
		}
	},
	# comment: binary message, no mosaics
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [],
			'message': unhexlify('D600000300504C5445000000FBAF93F7')
		}
	},
	# comment: simple message, 1 mosaic
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': 'SGEN27LSEJ7MVZYAAAAAAAAAAAAAAAAAAAAAAAA',
			'mosaics': [
				{'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000000001}
			],
			'message': 'It\'s some kind of magic, magic'
		}
	},
	# comment: simple message, 2 mosaics
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': 'Hello 👋'
		}
	},
	# comment: no message, 3 mosaics (out of order)
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'transfer_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x77A1969932D987D7, 'amount': 0x0000000000000002},
				{'mosaic_id': 0xD525AD41D95FCF29, 'amount': 0x0000000000000001},
				{'mosaic_id': 0x67F2B76F28BD36BA, 'amount': 0x0000000000000003}
			]
		}
	}
]
