from binascii import unhexlify

SCHEMA_NAME = 'SecretProofTransactionV1'


transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'secret_proof_transaction_v1',
			'recipient_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256',
			'proof': unhexlify('9A493664')
		}
	}
]
