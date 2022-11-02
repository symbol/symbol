SAMPLE_ADDRESS = 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'
SCHEMA_NAME = 'SecretLockTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'secret_lock_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'hash_algorithm': 'sha3_256'
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'secret_lock_transaction_v1',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'duration': 100,
			'secret': '59CC35F8C8D91867717CE4290B40EA636E86CE5C000000000000000000000000',
			'hash_algorithm': 'hash_160'
		}
	}
]
