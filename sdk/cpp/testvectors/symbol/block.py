
blocks = [  # pylint: disable=duplicate-code
	# normal block, no transactions
	{
		'schema_name': 'NormalBlockV1',
		'descriptor': {
			'type': 'normal_block_v1',
			'height': 759180,
			'difficulty': 10000000000000,
			'generation_hash_proof': {
				'gamma': '61F452701D1A1DD50406DDD98EC2E76F939F79A39243A1095F486835AAB7FB42',
				'verification_hash': '2C947E8F590D97B7578802BEF392E1D1',
				'scalar': '70D7791DC59772F69909F6387B3F2EBBD1FAAEFC71487043039A1B79319D730E'
			},
			'previous_block_hash': '4FC9BB3EF6799E98193253CE89F6D80C5424FAE0E194EF8279C27CD776F4E204',
			'transactions_hash': 'BC2EDB6C9E0FD86BEF9CBB2C7E85D761834D5E211718D7FFB0F3A605341808A1',
			'receipts_hash': '7AB373A6793CB7B9532DD5413D00DB92AB5F676BCF21BA847678A5AC59457E22',
			'state_hash': '3E39A5626FE62867C1E392322A7EAA5CCABAB38AD2E363E30430989178AAF9F6',
			'beneficiary_address': 'TCG6OD2R33SDNECORUGUD6W6BVEXLCMBFE62WCA',
			'fee_multiplier': 4807,
			'transactions': [
			]
		}
	},
	# nemesis block, single transaction
	{
		'schema_name': 'NemesisBlockV1',
		'descriptor': {
			'type': 'nemesis_block_v1',
			'height': 765000,
			'difficulty': 10101010101010,
			'generation_hash_proof': {
				'gamma': '31EDB0B110FB7116BD835CFE9206EDC6033C466DE213F5F28B2619A74DF40224',
				'verification_hash': '59DE3A1EBE9B8F4E78B00AEBC6BF122C',
				'scalar': 'D846AF0FC2A123F7012FDF882F73476939A620B61A07FC8FA321229664B79507'
			},
			'previous_block_hash': '491CB2272CAF0CD4F0EBA918A5AD4E2F9BFF9C6B0BF183BC461B6E9B756CF435',
			'transactions_hash': '0000000000000000000000000000000000000000000000000000000000000000',
			'receipts_hash': '691B47DD6DFC468883A48E73FFD22FCBA141A2371AD09376357E547C55C5C24C',
			'state_hash': '947B5EFB1C5FAF95BF294A6E020830203CFB9CCE6597E6ACF3C16C06CCAEB873',
			'beneficiary_address': 'TD72IGCQRI5QELXTJEP4QJKN5N7MH25GLNJN4HI',
			'fee_multiplier': 7867,
			'voting_eligible_accounts_count': 14,
			'harvesting_eligible_accounts_count': 478,
			'total_voting_balance': 116842289054626,
			'previous_importance_block_hash': 'DBDC925119781952C316DE84B87DAC640F64B899E191A7190EBCFDDED482A75D',
			'transactions': [
				{
					'type': 'transfer_transaction_v1',
					'recipient_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
					'mosaics': [
						{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
						{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
					],
					'message': 'Hello 👋'
				},
			]
		},
	},
	# importance block with 2 txes, one of txes is aggregate
	{
		'schema_name': 'ImportanceBlockV1',
		'descriptor': {
			'type': 'importance_block_v1',
			'height': 765360,
			'difficulty': 10101010101010,
			'generation_hash_proof': {
				'gamma': '42C7BE9082B10689005D62F85824B44D2AEA3444FAC5B66C09A80EC357B568F7',
				'verification_hash': 'B710A9B2447DF32800BA3ADADD58B4E8',
				'scalar': '035B2CD3F6810540FB34C5F4500CFD0550EBF54EDC42B7F7622069B4A0AC920C'
			},
			'previous_block_hash': '12D82BCC77171D8534629100B7FA10C2F0BF866B53542FAAECC421F4C8A9BB2A',
			'transactions_hash': '0000000000000000000000000000000000000000000000000000000000000000',
			'receipts_hash': 'B72E10BC0AF282BE231EA7B434C331F4597494F2C9E805F40BA4CDFBF3C8F619',
			'state_hash': 'BE371A51908393A47B6E69E75B80CA9017E12CDCE87A67FC811BD58D6437657B',
			'beneficiary_address': 'TCGJD3C6PGBW3UOFJHR2VLVM7UBPNGRWJ6AJXMQ',
			'fee_multiplier': 0,
			'voting_eligible_accounts_count': 14,
			'harvesting_eligible_accounts_count': 479,
			'total_voting_balance': 116887594468987,
			'previous_importance_block_hash': '77C5CD0D6B7BD6A0E62FC3910A13D764848C5DCECF176E04FA700E033F045753',
			'transactions': [
				{
					'aggregate': {'type': 'aggregate_bonded_transaction_v1'},
					'embedded': [
						{
							'type': 'transfer_transaction_v1',
							'recipient_address': 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I',
							'mosaics': [
								{'mosaic_id': 0x57701A9B6E746988, 'amount': 0x0000000000000064}
							],
						},
						{
							'type': 'mosaic_supply_change_transaction_v1',
							'mosaic_id': 0x00000B39C6051367,
							'action': 'increase',
							'delta': 0xB
						}
					],
					'num_cosignatures': 2
				},
				{
					'type': 'transfer_transaction_v1',
					'recipient_address': 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ',
					'mosaics': [
						{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
						{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
					],
					'message': 'Hello 👋'
				},
			]
		},
	}
]
