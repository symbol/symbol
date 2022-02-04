from binascii import unhexlify

from symbolchain import nc


def generate(factory):
	descriptors = {
		'MultisigTransaction_1': {
			'signature': nc.Signature(
				'23F098A01DB14157BED1705E1D5572E6623546CC9BC548B06D7CDAEF2550F70F'
				'92D1FB781B5110F1AA01BA90FFA3ED76A37E6454B6DF14D2D871E87F2E10FF01'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '1DD41E86DA2147F657CFE1D9495E72ABCFAF6332151225F0F0A840F21E0548F5',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'account_key_link_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
				'fee': 0x000000000001B669,
				'link_action': 'unlink',
				'remote_public_key': 'A393E06606A15A4F121D493D50801BB59784882D66FAD10C5E33289E09D7ED23',
			})),
			'cosignatures': [],
		},
		'MultisigTransaction_2': {
			'signature': nc.Signature(
				'61385B55BBD117B351667F21276035391BBD0425FB57AF9CE12A89A920BD0F85'
				'077772C0E487276C8F1A6BEB67DA4A4F22986A43DDD964D248AD9A0B093B9C08'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '1DD41E86DA2147F657CFE1D9495E72ABCFAF6332151225F0F0A840F21E0548F5',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'mosaic_definition_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
				'fee': 0x000000000001B669,
				'mosaic_definition': {
					'owner_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
					'id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
					'description': b'precious vouchers',
					'properties': [
						{'property_': {'name': b'divisibility', 'value': b'2'}},
						{'property_': {'name': b'initialSupply', 'value': b'1000'}},
						{'property_': {'name': b'supplyMutable', 'value': b'true'}},
						{'property_': {'name': b'transferable', 'value': b'true'}}
					],
					'levy_size': 0x50,
					'levy': {
						'transfer_fee_type': 'percentile',
						'recipient_address': 'TDALKDARRGRR7AQ4MFCGMM2IATJKTYU4H2IBE6HS',
						'mosaic_id': {'namespace_id': {'name': b'lizard'}, 'name': b'people'},
						'fee': 0x000000000001B6E6,
					}
				},
				'rental_fee_sink': 'TA6FN6JO6IJ5KZBICGM7MRRR53GUCBRGEWUEKT4D',
				'rental_fee': 0x0000000028697580,
			})),
			'cosignatures': [],
		},
		'MultisigTransaction_3': {
			'signature': nc.Signature(
				'5A98CBA099A2A35ABAC68FEBE468E9E58860E3FF10E7D78B89050833D91ABF27'
				'9B99CB947731E670C1BBC0FCA93179395916C9EB73FA357A8DE211B9F4AB0E0A'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '1DD41E86DA2147F657CFE1D9495E72ABCFAF6332151225F0F0A840F21E0548F5',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'mosaic_supply_change_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
				'fee': 0x000000000001B669,
				'mosaic_id': {'namespace_id': {'name': b'banksters'}, 'name': b'bailout'},
				'action': 'decrease',
				'delta': 0x00000000001DCD65,
			})),
			'cosignatures': [],
		},
		'MultisigTransaction_4': {
			'signature': nc.Signature(
				'F9E23236D69C3F02A0EE346DC3F389283FBD73B6F7E068D282130751EF5A1AD9'
				'1BD92D1776B1CB4A02F0F816DA4776048F315DC04A5198CD3B239522148DAC0D'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '1DD41E86DA2147F657CFE1D9495E72ABCFAF6332151225F0F0A840F21E0548F5',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'namespace_registration_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
				'fee': 0x000000000001B669,
				'rental_fee_sink': 'TD2ZK7SIVEILCRU2QLTVGUOFUEXVDBVSHKA4EXFC',
				'rental_fee': 0x0000000028697580,
				'name': b'controlled',
				'parent_name': b'state',
			})),
			'cosignatures': [],
		},
		'MultisigTransaction_5': {
			'signature': nc.Signature(
				'D2A5610E8D128A1B3B6F805EECB98CE2660B78DF6B523DFC8B1AEB51D86AD8A3'
				'06E8B8776CD3BB00E0197D265C171BFB4DCA2F6BA1216C8E82A2EFE28785DF06'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '1DD41E86DA2147F657CFE1D9495E72ABCFAF6332151225F0F0A840F21E0548F5',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'transfer_transaction',
				'version': 0x2,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
				'fee': 0x000000000001B669,
				'recipient_address': 'TA2BWO57KUJJKHBMPBE7MBQ3QX2COYLXLVVNH4IY',
				'amount': 0x00000000004C4B40,
				'message_envelope_size': 0x48,
				'message': {
					'message_type': 'encrypted',
					'message': unhexlify(
						'DCD66E9EF460A5AC2518FFB9C61B15148E0DBBC6CA2861C96BA81CBDDFF1F0DF'
						'D2082C55F8736A008569DA3B98C3F957E1621726BA6910A06EB7FCDABCA4BEB8'),
				},
				'mosaics': [
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'almost'}, 'name': b'nonfungible'},
							'amount': 0x0000000000000002
						},
					},
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
							'amount': 0x000000000005464E
						},
					},
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
							'amount': 0x000000000000007B
						},
					}
				],
			})),
			'cosignatures': [],
		},
		'MultisigTransaction_6': {
			'signature': nc.Signature(
				'AE0CC75C7A72FEE040C5B5EC2D396D40811473EFF805863A24664BB09411BD69'
				'FA2968B0159CBD18201D9DBF400EF3FEE1154AFC0449DF1D450A54EC2E33F00F'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '1DD41E86DA2147F657CFE1D9495E72ABCFAF6332151225F0F0A840F21E0548F5',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'multisig_account_modification_transaction',
				'version': 0x2,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'D9F85BADEC4CB94B40DC756726307C84A2C438D6BB6CB35358E8101C2EF74759',
				'fee': 0x000000000001B669,
				'modifications': [
					{
						'modification': {
							'modification_type': 'add_cosignatory',
							'cosignatory_public_key': 'B9E556F62229F20F020D84E0C2DD2814D099F87F58AB76AE772C70BEDC952723',
						}
					},
					{
						'modification': {
							'modification_type': 'add_cosignatory',
							'cosignatory_public_key': '06ABFE658E828ED96466200CB6BEAEDAD092E09531E07D283E7C29B335942940',
						}
					},
					{
						'modification': {
							'modification_type': 'delete_cosignatory',
							'cosignatory_public_key': '8C23F9E22DA269C56B54269DFC36676DD559FAA65ABA605340DF2C2A5F75CC0C',
						}
					},
				],
				'min_approval_delta': 0x1,
			})),
			'cosignatures': [],
		},
		'MultisigTransaction_7': {
			'signature': nc.Signature(
				'2F9088E93547AD783946547DBAB29505B1ED9FB2466AEBE135E03B9E7373CCA4'
				'37D865C674354D4EBC3FF79113C33ABCB4F714215B4AD856F6595BA845D01202'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '03CEEBAE7CA845F78494E46CD681F3A62CC73B692073D5A2AB44C54EE7EDECF0',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'account_key_link_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
				'fee': 0x000000000001B669,
				'link_action': 'unlink',
				'remote_public_key': '618007CBA7AF1A2406726443918D21DD0C74FFF9E95A95439FA34526BE4EFAC1',
			})),
			'cosignatures': [
				{
					'cosignature': {
						'signature': nc.Signature(
							'319BF1C3DD723811966E3BB9AC75D6374DF99F21AC683B4C55F8BEF0E4AF58C3'
							'91FF466A16D935900371B371F68CC0FF91CBB85558FFFAB4D8E0DF1832F0BC0A'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E241,
						'signer_public_key': '2EBB13C69338960C9AE138FC9D5BE1B2AE376170957FC9B7CBEA724028218194',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '949A87A8D8844BF731EE9579AA5AEA0724721476D5DF0E7D04E1732D475C51D4',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					}
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'24C71CBC998AA6D480D8053527973212CA85FC7BA01AB16362CBC77555A6CD7B'
							'54B1A5AE48277C4CAEA1147476A7D54AAF0FA9A8DD2E417EF3DB22BCFBCF070E'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E243,
						'signer_public_key': 'E163498D266219FF5C0078B456361925FB84DBEE23CCDCD37904A57B73788825',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '949A87A8D8844BF731EE9579AA5AEA0724721476D5DF0E7D04E1732D475C51D4',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					}
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'36AB63A865E96BEB153B650306670AD7F748B7B6373C3E66D4E11886672218A2'
							'D16FFD28EC0E99D74219ED0AC3DD6AAA8A252A203D9DC995814C54B2BB59D20B'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E242,
						'signer_public_key': '08E7C2B84DE60490E8F4C1545C679B25751668FAC921720E4D2718EAB966D3D0',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '949A87A8D8844BF731EE9579AA5AEA0724721476D5DF0E7D04E1732D475C51D4',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				}
			]
		},
		'MultisigTransaction_8': {
			'signature': nc.Signature(
				'3C3AF012CD75ED159E023C3CC13760424776EDD06CBC73CB253543FEB717C23D'
				'EF7A88420ACCF0EF5E8CDB4DA5E592BDB68F31365356C56AF361C0CE20C78201'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '03CEEBAE7CA845F78494E46CD681F3A62CC73B692073D5A2AB44C54EE7EDECF0',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'mosaic_definition_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
				'fee': 0x000000000001B669,
				'mosaic_definition': {
					'owner_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
					'id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
					'description': b'precious vouchers',
					'properties': [
						{'property_': {'name': b'divisibility', 'value': b'2'}},
						{'property_': {'name': b'initialSupply', 'value': b'1000'}},
						{'property_': {'name': b'supplyMutable', 'value': b'true'}},
						{'property_': {'name': b'transferable', 'value': b'true'}}
					],
					'levy_size': 0x50,
					'levy': {
						'transfer_fee_type': 'percentile',
						'recipient_address': 'TA3I3M6KEOM477UERTO72TX2FXHBSCI5UH757SY2',
						'mosaic_id': {'namespace_id': {'name': b'lizard'}, 'name': b'people'},
						'fee': 0x000000000001B6E6,
					},
				},
				'rental_fee_sink': 'TANNN22J3OL7TOJP3HUCQR2533JDAQF4GDG5PGVD',
				'rental_fee': 0x0000000028697580,
			})),
			'cosignatures': [
				{
					'cosignature': {
						'signature': nc.Signature(
							'576D5309FA7B455DA460C092FBCCAB17E94199B626DE5397DBF0798EE5D4629D'
							'D2ED3B391409FE9E7E66E56899C7602E504DC2DF5290DCC84CFBA6D63DA7B00C'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E242,
						'signer_public_key': '9EEF729124A0C5AE28F71C98F8D27029568522D2A2B9CDE9C18285509FC9B723',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '7886B89A4A4B49C099F9148264417B4FD833ACEDD89164C893D1C38AB8AFA76D',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					}
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'EBB06C164307F7232228CA3AEE67786FF3D2F1B4257A632D25E4BAF25E5FB2AB'
							'701D0F3B6198BDE14ADECAAEFAA9B144F68F0265257052A4F4CAB0B6D39D890C'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E241,
						'signer_public_key': 'C79E70B911CCBE11FEB1EAEDEE983830203AF688C0964365C3183189E5389730',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '7886B89A4A4B49C099F9148264417B4FD833ACEDD89164C893D1C38AB8AFA76D',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'D07AB5E77485B6F372D068FF7022623389D0C1498C93E825289D4C1CEA6B5E82'
							'C0B106F07128F758DBA693EF0504C265C7E49E4FCF6E64F680453C32FE5AE207'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E243,
						'signer_public_key': '69FBDB45EBC7D3D9690135A86E786A2C7F0E0834558DEDF38CA7723D6C5DAE71',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '7886B89A4A4B49C099F9148264417B4FD833ACEDD89164C893D1C38AB8AFA76D',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ'
					}
				}
			]
		},
		'MultisigTransaction_9': {
			'signature': nc.Signature(
				'9B98E2AD79A6E8D97F4747A1945E7DFBFC2C7E89C698D32DA81BFB8D436C29E5'
				'A2870432C0EA27AFAD7C4523D764281D8DF2D20105AD852B8C8F3332748C1A03'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '03CEEBAE7CA845F78494E46CD681F3A62CC73B692073D5A2AB44C54EE7EDECF0',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'mosaic_supply_change_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
				'fee': 0x000000000001B669,
				'mosaic_id': {'namespace_id': {'name': b'banksters'}, 'name': b'bailout'},
				'action': 'decrease',
				'delta': 0x00000000001DCD65,
			})),
			'cosignatures': [
				{
					'cosignature': {
						'signature': nc.Signature(
							'28782899B6F6EB37440342DA95C4369188C0F9F801F17F02B8A633A990069AD3'
							'77E0A99CEFBDA8076EC2EA7638AF81560DAD2BCB1C9C4A141F23979F3A7DEF07'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E241,
						'signer_public_key': '2197EFBAC3BBC06CF83AAA11FBCF0EC8177C160595A2E9FF1A61AD52514DFEE9',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': 'FFD674D46C123F2FC76645DE48DC863703FDED373EA3EFF3886CAADBB83B5259',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					}
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'805A2F0F1A0B7332F25F395BD717A9D64241556886A373345179E3CABC24C843'
							'B3A456DFFDDBF0825F49B4C0448C5718B746F89A6519DB50BF00B4DC66689C04'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E243,
						'signer_public_key': 'D2A19E18CF0F1526891BE9E6AFC3C891E5FC39CDA32ACC4D2A4CCF7AD212CB76',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': 'FFD674D46C123F2FC76645DE48DC863703FDED373EA3EFF3886CAADBB83B5259',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					}
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'60F1030AEE2AB7AE8BBDEE39B9AFC68F2941F86CD9B311754F3D7F7205622643'
							'76BBFE88FD21B660831FFB9211DF735CBB6B04EDC0A099522B6074354F67A20E'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E242,
						'signer_public_key': '58C51300675048A516710AC00C5A82F02EEA22BBD705DDD714F83D694B50A5A4',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': 'FFD674D46C123F2FC76645DE48DC863703FDED373EA3EFF3886CAADBB83B5259',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					}
				}
			]
		},
		'MultisigTransaction_10': {
			'signature': nc.Signature(
				'DDB06B7CA139BAB36D63840076AB6136496C33ADDDC434D725E79A24FF79CA1D'
				'23B1EF5AA55A837C799A2EC25B009C49D2B9C61CE55E9C0C2EB6D10EBFCB7B03'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '03CEEBAE7CA845F78494E46CD681F3A62CC73B692073D5A2AB44C54EE7EDECF0',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'namespace_registration_transaction',
				'version': 0x1,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
				'fee': 0x000000000001B669,
				'rental_fee_sink': 'TA7AICXV3PMLTTRHLACOBRZIJHMAPV3XOQ2QOPQW',
				'rental_fee': 0x0000000028697580,
				'name': b'controlled',
				'parent_name': b'state',
			})),
			'cosignatures': [
				{
					'cosignature': {
						'signature': nc.Signature(
							'0E4FD0C95781465C69AF6DC837A7AB7EFB0ECA1EDC4EFBF20E8DC5EDB73266C8'
							'E9A6C9A900EAF12DF71AC62365B696B8CA8F4E7B2D3E75190003E794DC316502'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E243,
						'signer_public_key': '962BACA5E6E5E6D332C4DE804D51BF257D07421F05C367951ED183684B110912',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '147E5CCC0CB6C44E54747299AEF49DF5601501C3555C81CF69686F5A254C5E6C',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'BCDC004CD7884ED92F9751B30EB4508BA216B4C88A9879AC2DF05276589C34D7'
							'6C68085869A530E920C66F8C07F16C714FE4140323DC249BEC3091E5B8C3440C'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E241,
						'signer_public_key': '3F090F210B4961195012CFE7A731D43622353C6C9FA42DE8B4F9938164E61325',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '147E5CCC0CB6C44E54747299AEF49DF5601501C3555C81CF69686F5A254C5E6C',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E242,
						'signer_public_key': '9C12D71F946A19216E9E552541320D16D6E5AC4E8812FBF4E4743715837FBAD6',
						'signature': nc.Signature(
							'756F5EA660B682380359643110D31D3327CA09EB73C5336A76229F4D6CE454F3'
							'2601C19BA4F78E0CAC4088B74906E51F9BE6CC5C5726CB25053FE87018928601'),
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '147E5CCC0CB6C44E54747299AEF49DF5601501C3555C81CF69686F5A254C5E6C',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				}
			],
		},
		'MultisigTransaction_11': {
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '03CEEBAE7CA845F78494E46CD681F3A62CC73B692073D5A2AB44C54EE7EDECF0',
			'signature': nc.Signature(
				'B5335771AAD5EDA8D534174FF496F41033DD8C0FBB77DA05855547F111357A60'
				'7EA7D3875BDAA3566D876F5C9BEEEDDB3A6572F76945FDE5D6933AA75DD3B007'),
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'transfer_transaction',
				'version': 0x2,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
				'fee': 0x000000000001B669,
				'recipient_address': 'TD7UXAKTXBMM2SSATFO4IGNY4MIEOXZWBQ22SVVQ',
				'amount': 0x00000000004C4B40,
				'message_envelope_size': 0x48,
				'message': {
					'message_type': 'encrypted',
					'message': unhexlify(
						'2821A904321DCF8046596B98311419675CE01E47E81AB2EC1789B271417852C4'
						'40DBB1469A71995C4DE4C144B0D340C5B3EA95C2FA6A02BB45694000101C5D6F'),
				},
				'mosaics': [
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'almost'}, 'name': b'nonfungible'},
							'amount': 0x0000000000000002,
						},
					},
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
							'amount': 0x000000000005464E
						}
					},
					{
						'mosaic': {
							'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
							'amount': 0x000000000000007B
						},
					}
				],
			})),
			'cosignatures': [
				{
					'cosignature': {
						'signature': nc.Signature(
							'0F8CAC8EA74FFD3CB7522D8693E46F1B3FDAEAF6508829F72CD63EFBDE49D203'
							'0BFF9BF822277F3F54FBA7D899984776BF17CBBDA8DC00732A22A098101C4308'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E241,
						'signer_public_key': '32C2757A8D591C7958B8D1C9D755525BF12992DC3E2290AF856DBEEE308244F5',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '7881335F7DC4E5BD5657E81FFDF7DC89C0706E42A9C281E82060740F87488B0E',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'F0AD594387C94621214D0007F135915FA6FD0A89B42C527CE73D0254482DB1D8'
							'7EAD75860EDB8D036FB08D547F35AE0DE262301146EECCCC89596EE0D9050D06'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E242,
						'signer_public_key': '6AB35031B4D5294F9A6D00CD7B95E7D8119C1DBF27B5C0BE16EA450D9B7A3758',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '7881335F7DC4E5BD5657E81FFDF7DC89C0706E42A9C281E82060740F87488B0E',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'EE89594EA3B1C08379B575A1F0C73E9205BC25B1DB9BFF6195F912F323D5B2CD'
							'2A176D9FC7F3DCA54DA0FBF404B5D1DED8B5F766D7010D7A33F0432A36A94004'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E243,
						'signer_public_key': '0459F033D5C2359AA8F5AE3CA82134B0F8E8DE019BCDDB080907441AE7EFB49D',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '7881335F7DC4E5BD5657E81FFDF7DC89C0706E42A9C281E82060740F87488B0E',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				}
			],
		},
		'MultisigTransaction_12': {
			'signature': nc.Signature(
				'7D2FBEAFCF1714FEA039C7FC26838CEFA89AB788CB67FBF5E9999BABF8DAA717'
				'3A02EDB35B34F2563EAF2A1A247E17DC738921B9C4809D784902C8413370C803'),
			'type': 'multisig_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '03CEEBAE7CA845F78494E46CD681F3A62CC73B692073D5A2AB44C54EE7EDECF0',
			'fee': 0x000000000001B669,
			'inner_transaction': factory.to_non_verifiable_transaction(factory.create({
				'type': 'multisig_account_modification_transaction',
				'version': 0x2,
				'network': 'testnet',
				'timestamp': 0x0001E240,
				'signer_public_key': 'E57D899F72399561124F644B97A3948B6E215DB97F0922A757F4D98B50F636F6',
				'fee': 0x000000000001B669,
				'modifications': [
					{
						'modification': {
							'modification_type': 'add_cosignatory',
							'cosignatory_public_key': '1058F3D51419F8706E351855B70D85C75F63A25DEE707C4D7DAA66888DC8A3E1'
						}
					},
					{
						'modification': {
							'modification_type': 'add_cosignatory',
							'cosignatory_public_key': '2F4294305A2698932CBC097F4F05F58C0204B4713FD841FF2296600D94D46595',
						}
					},
					{
						'modification': {
							'modification_type': 'delete_cosignatory',
							'cosignatory_public_key': '3D19FDC26B5196B3EC6FBDDA1527C6520A879CBCB0FEE2F36113AEA10774DD6E',
						}
					}
				],
				'min_approval_delta': 0x1,
			})),
			'cosignatures': [
				{
					'cosignature': {
						'signature': nc.Signature(
							'FE23A3B37CFAE6F51335ACFBCF7BD4CC8AE05D9231D5043F6F91D8C53B2E5D29'
							'6658AC36E9ABBCEE092F4EF6EAE1B2448478046DDB36AD99E2FDE8CB12BA500C'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E242,
						'signer_public_key': 'DDC7AEBB2C5659D90983CF74DD77517F92F72A8FA081E3CE72C740F43EF35B54',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '2636F3D2B4506C356308C56876CC3670F6EF37C8F2A1D0E670BDA3605F192DA2',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'E9E04C4C86970A60D7FDD7D8CDFB4E1C6E34C8DD13AB3124394159B6A292EA71'
							'802F95683E33E138550263D3EC16410717EA5C6AE4DD24EE4F9DD36B40000903'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E241,
						'signer_public_key': '1E463D5AC9406A9E882E8EC3FF053BEBB84CE309443AEEB13B5183E32DBD5920',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '2636F3D2B4506C356308C56876CC3670F6EF37C8F2A1D0E670BDA3605F192DA2',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				},
				{
					'cosignature': {
						'signature': nc.Signature(
							'BEDF2B564DD006F5B9FCE1E29F15DD7F195014740A914A19CC72CA3249EAE57E'
							'F53DFA003F776DC12AAC89FEFBB5EDE2A82BD8D5844294A90EA94EF31C08130E'),
						'version': 0x1,
						'network': 'testnet',
						'timestamp': 0x0001E243,
						'signer_public_key': '5C19037E751950CEBCC8B7F14AF02E82D7F491D28C9064D5939D62A258DCCD4D',
						'fee': 0x000000000001B669,
						'multisig_transaction_hash': '2636F3D2B4506C356308C56876CC3670F6EF37C8F2A1D0E670BDA3605F192DA2',
						'multisig_account_address': 'TDWDXPEZFJLF7OXKI36Y7DCG2JDH3ZHOHR6S2GLQ',
					},
				}
			],
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
