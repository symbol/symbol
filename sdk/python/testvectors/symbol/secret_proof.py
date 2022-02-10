from binascii import unhexlify

recipes = {
	'schema_name': 'SecretProofTransaction',
	'descriptors': [
		{
			'type': 'secret_proof_transaction',
			'recipient_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256',
			'proof': unhexlify('9A493664')
		}
	]
}
