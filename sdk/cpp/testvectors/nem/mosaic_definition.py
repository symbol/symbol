SCHEMA_NAME = 'MosaicDefinitionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	# comment: without supply
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'mosaic_definition': {
				'owner_public_key': '875BD953CB8EC0BDEAA01552E390B5E59DEAAD44D81BD7DEAF0C332F99AEECE8',
				'id': {
					'namespace_id': {'name': b'alice.vouchers'},
					'name': b'alice\'s gift vouchers',
				},
				'description': b'precious vouchers',
				'properties': [
					{'property_': {'name': b'divisibility', 'value': b'3'}},
					{'property_': {'name': b'initialSupply', 'value': b'0'}},
					{'property_': {'name': b'supplyMutable', 'value': b'false'}},
					{'property_': {'name': b'transferable', 'value': b'true'}}
				]
			},
			'rental_fee_sink': 'TDX5YX2NJUSWXEKJ4UQN3WXUY3SCCAGWHHFJ3B5J',
			'rental_fee': 0x0000000028697580
		}
	},
	# comment: non-zero supply
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'mosaic_definition': {
				'owner_public_key': 'F53B19CB1CAB394E22E3DDFA6D9B42DA87F37EB517EFA214433071E5619F898D',
				'id': {
					'namespace_id': {'name': b'alice.vouchers'},
					'name': b'alice\'s gift vouchers',
				},
				'description': b'precious vouchers',
				'properties': [
					{'property_': {'name': b'divisibility', 'value': b'3'}},
					{'property_': {'name': b'initialSupply', 'value': b'345678'}},
					{'property_': {'name': b'supplyMutable', 'value': b'false'}},
					{'property_': {'name': b'transferable', 'value': b'true'}}
				]
			},
			'rental_fee_sink': 'TCO5WSTHXII62V3MYWKBD7GOMCDRX35TFOZEX3BD',
			'rental_fee': 0x0000000028697580
		}
	},
	# comment: supplyMutable
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'mosaic_definition': {
				'owner_public_key': '69AF8763FEECEF35E0AEF44A202EFAC4936532C202784CF85CBCD16BFC45F119',
				'id': {
					'namespace_id': {'name': b'state.sponsored'},
					'name': b'terrorism',
				},
				'description': b'precious vouchers',
				'properties': [
					{'property_': {'name': b'divisibility', 'value': b'2'}},
					{'property_': {'name': b'initialSupply', 'value': b'1000'}},
					{'property_': {'name': b'supplyMutable', 'value': b'true'}},
					{'property_': {'name': b'transferable', 'value': b'true'}}
				]
			},
			'rental_fee_sink': 'TARPSQFPJL6A2ORAQJ46GOZUDPNYVQJKGVT2NMG7',
			'rental_fee': 0x0000000028697580
		}
	},
	# comment: absolute levy
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'mosaic_definition': {
				'owner_public_key': 'B5B93DDE2D05D21D0A14E6F60DB33E983B88A99FB203916D408AF5749A396960',
				'id': {
					'namespace_id': {'name': b'mass.surveillance'},
					'name': b'electronic',
				},
				'description': b'precious vouchers',
				'properties': [
					{'property_': {'name': b'divisibility', 'value': b'2'}},
					{'property_': {'name': b'initialSupply', 'value': b'1000'}},
					{'property_': {'name': b'supplyMutable', 'value': b'true'}},
					{'property_': {'name': b'transferable', 'value': b'true'}}
				],
				'levy': {
					'transfer_fee_type': 'absolute',
					'recipient_address': 'TCUTCNM64Y6Q4VB4OTEHBQT2ZKUY3CUYRVHCTIZ3',

					'mosaic_id': {
						'namespace_id': {'name': b'reptilians'},
						'name': b'united',
					},
					'fee': 0x00000000037706A6,
				}
			},
			'rental_fee_sink': 'TD3M6P3CENCDVIC2GRJLQPLQXBX6MOB6FBQZOXM2',
			'rental_fee': 0x0000000028697580,
		}
	},
	# comment: percentile-based levy
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_definition_transaction_v1',
			'mosaic_definition': {
				'owner_public_key': 'D7F44870D24E7626DB24591452A2F7ECF6650B0D41D3BCB4FB4BA11B063B80AC',
				'id': {
					'namespace_id': {'name': b'mass.surveillance'},
					'name': b'electronic',
				},

				'description': b'precious vouchers',
				'properties': [
					{'property_': {'name': b'divisibility', 'value': b'2'}},
					{'property_': {'name': b'initialSupply', 'value': b'1000'}},
					{'property_': {'name': b'supplyMutable', 'value': b'true'}},
					{'property_': {'name': b'transferable', 'value': b'true'}}
				],
				'levy': {
					'transfer_fee_type': 'percentile',
					'recipient_address': 'TDAQGCFP4TR2U33WBXJEDUHH6OWV3T4YXWHSH46A',

					'mosaic_id': {
						'namespace_id': {'name': b'lizard'},
						'name': b'people',
					},
					'fee': 0x000000000001B6E6,
				}
			},
			'rental_fee_sink': 'TA2ZI764OYY233XIKTETFPGCC2HKORQSHTCUGCHX',
			'rental_fee': 0x0000000028697580
		}
	}
]
