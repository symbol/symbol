from symbolchain import nc


def generate(factory):
	descriptors = {
		'Cosignature_1': {
			'signature': nc.Signature(
				'AC121CFCE870B53DCF77F660424C45E5413463E39F06988D42ED9A17E190397D'
				'7DB9A690F7A89FACBECEAE77969806991B57F8C718BD28A24F55D6B589551608'),
			'type': 'cosignature',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '143309AE2DD476099D24B7259D4F86B206584B532CD2C9612C66451F54D28D96',
			'fee': 0x00000000005B8D80,
			'multisig_transaction_hash': 'F51C1E0B91C34AB3E54B8B6FD2BA6209254D47E0AF0C74C90BB2B99100B94829',
			'multisig_account_address': 'TBT7GACQQLYXUFBSQCUHXXWQMSRDAJPACTNJ724W',
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
