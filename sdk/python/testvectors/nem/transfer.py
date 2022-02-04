from binascii import unhexlify

from symbolchain import nc


def generate(factory):
	descriptors = {
		'TransferTransaction_1': {
			'signature': nc.Signature(
				'B2019592ECC6FCC62A8ED7343F69A9847E0515943EFB1B7AE2CB762C179A3BA2'
				'433E0A20CB26138761C7D548A2E7A2FA67D163880C2D9C44BFF779C9A21C1007'),
			'type': 'transfer_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x0
		},

		'TransferTransaction_2': {
			'signature': nc.Signature(
				'E52EA9D47B3139039BD4F06F70FE0FF74F44B378EB09F4731D8540D8019A244A'
				'4C0B14C994FAD0ABCD6681EB09FD8E4AFA0BD9A4453116D15D377AA94B7AEA0B'),
			'type': 'transfer_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x8,
			'message': {
				'message_type': 'plain',
				'message': '',
			}
		},
		'TransferTransaction_3': {
			'signature': nc.Signature(
				'22EF927FEFF0260C34E022D5FF885F8C442019CB168AAB47C14B6BCAC6C6DB5E'
				'9DB1DA0344C68930301DFA6AEB3890BD5A69C86E116694C615D1D55FDBF8C70E'),
			'type': 'transfer_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0xB,
			'message': {
				'message_type': 'plain',
				'message': unhexlify('0C3215'),
			}
		},
		'TransferTransaction_4': {
			'signature': nc.Signature(
				'CD650B5C92EDCE6F3BE7C90F0C3C02A4DF9E4970047A1438F848597F2C112465'
				'5BAC047D7D64E233E1B4AD6EBBE8AC9186BD6DF28D1A527405ABFC83D720A70D'),
			'type': 'transfer_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'642E756F0DBF4BE90EFCAB7AB42F297ED64EDC83E05062F8E49F6CAC61FBBA9F'
					'8D198B4B79A33096C58977C9134FAEE74307A6E55156BF6C2718E2B0D44BE050'),
			}
		},
		'TransferTransaction_5': {
			'signature': nc.Signature(
				'C55D66AE9167E173E7056146E36B17B9D70FE63D1CA3658DB140CF48FDA629E7'
				'12AEEA1A712B23A70BD618FCCD2ED84175424C1932EE45A840D148CC39415A0A'),
			'type': 'transfer_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x0000009858921E40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'3BD0346071C9801F1B58638794F548F32E0C5651C3D5BB3EEFAF106333244598'
					'C0B0AF2E3D80C3DAEF8E7ACEE94F67A6CA3213509E9E97594954E3ECBA7BCF9A'),
			}
		},
		'TransferTransaction_6': {
			'signature': nc.Signature(
				'385EFBDFE0191C24B3E85B8E5F8E1EB506AAADFF32F1D23DACE26557715608FA'
				'058A834510D8E9413556D06283BF54BE7B89407BB8CF9F9E06FD6C420D201802'),
			'type': 'transfer_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0xB,
			'message': {
				'message_type': 'plain',
				'message': unhexlify('0C3215'),
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				}
			]
		},

		'TransferTransaction_7': {
			'signature': nc.Signature(
				'D39D8C4C961D2A89A3EFE3F97F24DB628FCBF8D70E0CBE2C6D3E6A77E2DFF475'
				'A334A043F91E4C4ECAE0A887B12CC30DEEEEC2E4C57F12842C05A827431DB804'),
			'type': 'transfer_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'3F7FED92C755C444C492BD5DFD003A0F6F7E257A4DE7CC91E33EDD477F29C9BB'
					'5BD906BE5CC9E20B80A4F7C65C24727FAC4145AD42E055DBEDF516D96D386516'),
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				}
			]
		},
		'TransferTransaction_8': {
			'signature': nc.Signature(
				'AD3ECFE7C3C83EC2A05C281D8AB86A27048A9BAC29A956EA62B32390E81B0970'
				'DDF9C2808DD060625190078B19D80D7EE71140E9E0231862778D2189F428CF0E'),
			'type': 'transfer_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0xB,
			'message': {
				'message_type': 'plain',
				'message': unhexlify('0C3215'),
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'almost'}, 'name': b'nonfungible'},
						'amount': 0x0000000000000002,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
						'amount': 0x000000000000007B,
					}
				}
			]
		},
		'TransferTransaction_9': {
			'signature': nc.Signature(
				'4EA4FC45BFD12A21C1A134171E0DBE20182036C3CCF9B2B09EFAF8C448169F16'
				'0AD6738B8C8153CB518EED41B3210E1646711DA58510603DF84D2BB2FDF0FB00'),
			'type': 'transfer_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '94339ACC6299550ED8F077F58641CA31276E011172677DCC57851A8C9E6B2530',
			'recipient_address': 'TACQ6J4XXABJ4FRQ63ZHQ7PGDDTZCBJYK4ANOE36',
			'amount': 0x00000000004C4B40,
			'message_envelope_size': 0x48,
			'message': {
				'message_type': 'encrypted',
				'message': unhexlify(
					'48365F97EB76BAD136AC4630E792D1610B6591C9BF7DE0EA9D618FD7480AAE21'
					'A092BC8223F427B8263C7ED4EB665C945B569E42F48FF1A50A4CD20A3AF5173D'),
			},
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'almost'}, 'name': b'nonfungible'},
						'amount': 0x0000000000000002,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'mass.surveillance'}, 'name': b'electronic'},
						'amount': 0x000000000005464E,
					}
				},
				{
					'mosaic': {
						'mosaic_id': {'namespace_id': {'name': b'nem'}, 'name': b'xem'},
						'amount': 0x000000000000007B,
					}
				}
			]
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
