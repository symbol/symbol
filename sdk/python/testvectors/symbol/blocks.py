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
			'generation_hash_proof': {
				'gamma': 'AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55AA55',
				'verification_hash': '1234567890ABCDEF1234567890ABCDEF',
				'scalar': 'BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66BB66'
			},
			'previous_block_hash': '1122334411223344112233441122334411223344112233441122334411223344',
			'transactions_hash': '5566778855667788556677885566778855667788556677885566778855667788',
			'receipts_hash': '9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB',
			'state_hash': 'CCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFF',
			'beneficiary_address': SAMPLE_ADDRESS,
			'fee_multiplier': 100,
			'transactions': []
		},
		# comment: block with single transfer
		{
			'type': 'normal_block',
			'height': 6,
			'timestamp': 0x3344556677881122,
			'difficulty': 10**13,
			'generation_hash_proof': {
				'gamma': 'CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77CC77',
				'verification_hash': '567890ABCDEF1234567890ABCDEF1234',
				'scalar': 'DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88DD88'
			},
			'previous_block_hash': '3344112233441122334411223344112233441122334411223344112233441122',
			'transactions_hash': '5566778855667788556677885566778855667788556677885566778855667788',
			'receipts_hash': 'AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900AABB9900',
			'state_hash': 'EEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDDEEFFCCDD',
			'beneficiary_address': SAMPLE_ADDRESS,
			'fee_multiplier': 100,
			'transactions': [
				{
					'type': 'transfer_transaction',
					'recipient_address': SAMPLE_ADDRESS,
					'mosaics': [
						{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
						{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
					],
					'message': 'Hello 👋'
				}
			]
		},
		{
			'type': 'normal_block',
			'height': 7,
			'timestamp': 0x8877665544332211,
			'difficulty': 10**14,
			'generation_hash_proof': {
				'gamma': 'AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555',
				'verification_hash': '1234567890ABCDEF1234567890ABCDEF',
				'scalar': 'BBBB6666BBBB6666BBBB6666BBBB6666BBBB6666BBBB6666BBBB6666BBBB6666'
			},
			'previous_block_hash': '1111222233334444111122223333444411112222333344441111222233334444',
			'transactions_hash': '5555666677778888555566667777888855556666777788885555666677778888',
			'receipts_hash': '99990000AAAABBBB99990000AAAABBBB99990000AAAABBBB99990000AAAABBBB',
			'state_hash': 'CCCCDDDDEEEEFFFFCCCCDDDDEEEEFFFFCCCCDDDDEEEEFFFFCCCCDDDDEEEEFFFF',
			'beneficiary_address': SAMPLE_ADDRESS,
			'fee_multiplier': 100,
			'transactions': [
				{
					'aggregate': {'type': 'aggregate_complete_transaction'},
					'embedded': [
						{
							'type': 'transfer_transaction',
							'recipient_address': 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ',
							'mosaics': [],
							'message': 'Hello 👋'
						},
						{
							'type': 'transfer_transaction',
							'recipient_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
							'mosaics': [
								{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064},
								{'mosaic_id': 0xD525AD41D95FCF29, 'amount': 0x0000000000000001}
							],
							'message': unhexlify('D600000300504C5445000000FBAF93F7')
						}
					],
					'num_cosignatures': 3
				},
				{
					'type': 'transfer_transaction',
					'recipient_address': SAMPLE_ADDRESS,
					'mosaics': [
						{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
						{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
					],
					'message': 'Hello 👋'
				},
				{
					'type': 'secret_proof_transaction',
					'recipient_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
					'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
					'hash_algorithm': 'sha3_256',
					'proof': unhexlify('9A493664')
				}
			]
		}
	]
}
