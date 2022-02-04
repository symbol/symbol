from symbolchain import nc


def generate(factory):
	descriptors = {
		'AccountKeyLinkTransaction_1': {
			'signature': nc.Signature(
				'6C7275D9DD098B55754CA66AA409FA0E7387936BDE826D92CA1D93381064064B'
				'FDD02E85E9A182F24CE18E95EC6007987E5CA8EB635C0FA67F3B7AE9DAF27808'),
			'type': 'account_key_link_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '53DE7F9FC0A030F1DEEE91B5DD1BBF22AD991FDC7572E0F9B23B035A284B1081',
			'fee': 0x00000000005B8D80,
			'link_action': 'link',
			'remote_public_key': '6269E26026CECEFE640C3E0DE050CB9B3CFD279A0713CF00E16EDEF5D6C10EB9',
		},
		'AccountKeyLinkTransaction_2': {
			'signature': nc.Signature(
				'D47C3DF92E60992D3AE8BEAD93DFFE6B05718B2AB30B4D588D20C6BF781F8F5A'
				'B1B92FE8EA5A540B6F9BF97D05C61249F011465AA484EEF9F9DBEA1AEF188704'),
			'type': 'account_key_link_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': 'AFABE836FB0445902670EA190CEB28E1EB9D46AF05ECE026A4772505917E993C',
			'fee': 0x00000000005B8D80,
			'link_action': 'unlink',
			'remote_public_key': 'C3D4EAEB517BDDF22F21A2F7B61194D50666EDEBFE81B9118DB59ABE1D6E98E5',
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
