from binascii import unhexlify

SAMPLE_ADDRESS_ALIAS = 'TFIXOYLI2JBFPWAAAAAAAAAAAAAAAAAAAAAAAAA'
SAMPLE_ADDRESS = 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I'


block_recipes = {
	'schema_name': 'NormalBlock',
	'descriptors': [
		# comment: empty block
		{
			'type': 'normal_block',
			'height': 5,
			'timestamp': 0x1122334455667788,
			'difficulty': 10**14,
			#'generation_hash_proof': ,
			'previous_block_hash': '1122334411223344112233441122334411223344112233441122334411223344',
			'transactions_hash': '5566778855667788556677885566778855667788556677885566778855667788',
			'receipts_hash': '9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB',
			'state_hash': 'CCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFF',
			'beneficiary_address': SAMPLE_ADDRESS,
			'fee_multiplier': 100,
			'transactions': []
		}
	]
}
