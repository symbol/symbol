from symbolchain import nc


def generate(factory):
	descriptors = {
		'MosaicSupplyChangeTransaction_1': {
			'signature': nc.Signature(
				'E2C6DEED63EBF9747316B32438DFF99CA0239AE2D790E9C126C24D7502C25118'
				'9FE749A78AC2CF08AD32BF87597621AD47ECB095BED2BF7AE2EC19B2D8ACAC06'),
			'type': 'mosaic_supply_change_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': 'E6097B614ECB1CEA6EA03AFA207DD39E882223DAAC0B4331B18758E45B1DF679',
			'fee': 0x00000000066FF300,
			'mosaic_id': {
				'namespace_id': {'name': b'banksters'},
				'name': b'bailout'
			},
			'action': 'increase',
			'delta': 0x000000000005464E
		},

		'MosaicSupplyChangeTransaction_2': {
			'signature': nc.Signature(
				'A8E7266D1D59D8295EC5883B241CCF0D1B1DF645C571CBC77E2FF263CEC8FCC4'
				'C3AA46950A2F8E1BA52D26C01C21AA0490C4C84CF82BBE80453AE95791EF3300'),
			'type': 'mosaic_supply_change_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': 'C04A14C2E0A2455AC07C6A59B32038C7FBD826702DB581FA1290BC4C8C002317',
			'fee': 0x00000000066FF300,
			'mosaic_id': {
				'namespace_id': {'name': b'banksters'},
				'name': b'bailout',
			},
			'action': 'decrease',
			'delta': 0x00000000001DCD65,
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
