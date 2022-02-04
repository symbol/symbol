from symbolchain import nc


def generate(factory):
	descriptors = {
		'MosaicDefinitionTransaction_1': {
			'signature': nc.Signature(
				'630C3CBE07DD0A4A47056962A6C640AEA879ACEDAE94A912F855E443BF8B35C9'
				'285DF5798EA1325FD1357E13EB6A8353A441D8008853C3DF62A46A4D3EFCCA0A'),
			'type': 'mosaic_definition_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '875BD953CB8EC0BDEAA01552E390B5E59DEAAD44D81BD7DEAF0C332F99AEECE8',
			'fee': 0x00000000066FF300,
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
				],
				'levy_size': 0x0,
			},
			'rental_fee_sink': 'TDX5YX2NJUSWXEKJ4UQN3WXUY3SCCAGWHHFJ3B5J',
			'rental_fee': 0x0000000028697580
		},

		'MosaicDefinitionTransaction_2': {
			'signature': nc.Signature(
				'2142797DAEB7074AC00F032FE2CAE10CC35BB393CD3A133A91A7779768D7029E'
				'860C79795E31BDF2E4035F744E163BBC11F6B1566809F7BFF3578445C5C63D09'),
			'type': 'mosaic_definition_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': 'F53B19CB1CAB394E22E3DDFA6D9B42DA87F37EB517EFA214433071E5619F898D',
			'fee': 0x00000000066FF300,
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
				],
				'levy_size': 0x0,
			},
			'rental_fee_sink': 'TCO5WSTHXII62V3MYWKBD7GOMCDRX35TFOZEX3BD',
			'rental_fee': 0x0000000028697580,
		},

		'MosaicDefinitionTransaction_3': {
			'signature': nc.Signature(
				'84805287C15BA5B3055619DE86BEA8D4B65CE236371B3D362EE1181E536A5491'
				'F1DFF784CC390ABAF95EFA851C2290E44B3E2B2D5B4379F82D55B7307D13E205'),
			'type': 'mosaic_definition_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '69AF8763FEECEF35E0AEF44A202EFAC4936532C202784CF85CBCD16BFC45F119',
			'fee': 0x00000000066FF300,
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
				],
				'levy_size': 0x0,
			},
			'rental_fee_sink': 'TARPSQFPJL6A2ORAQJ46GOZUDPNYVQJKGVT2NMG7',
			'rental_fee': 0x0000000028697580
		},

		'MosaicDefinitionTransaction_4': {
			'signature': nc.Signature(
				'FDE6262170C31B7AFD736B2BD4871FAEC176F02C6626A1F67A3B295F129145E7'
				'89517C483FEA9E72FA08B808ED3A528B7FD514186E80FBF4E868CA7A84CE4309'),
			'type': 'mosaic_definition_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': 'B5B93DDE2D05D21D0A14E6F60DB33E983B88A99FB203916D408AF5749A396960',
			'fee': 0x00000000066FF300,
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
				'levy_size': 0x54,  # note: needs to be here, as it is not derived now...
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
		},

		'MosaicDefinitionTransaction_5': {
			'signature': nc.Signature(
				'24056CEC2CFCC0C834336BB19701B91FF9129E0F1EF440441D3FB400D69F285F'
				'B3BF5E8C6139D181ADD15D2918A623DBF130D14A3047A26C38DA72E9BD62AE08'),
			'type': 'mosaic_definition_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': 'D7F44870D24E7626DB24591452A2F7ECF6650B0D41D3BCB4FB4BA11B063B80AC',
			'fee': 0x00000000066FF300,
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
				'levy_size': 0x50,  # note: needs to be here, as it is not derived now...
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

	return {k: factory.create(v) for (k, v) in descriptors.items()}
