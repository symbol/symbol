from symbolchain import nc


def generate(factory):
	descriptors = {
		'NamespaceRegistrationTransaction_1': {
			'signature': nc.Signature(
				'7BB9FC2F39D56511D8CA4CDA6FDC555481C5C75B0A00BC6198260B607B4F5AEA'
				'185413F98183646DE6ED208F22DB4AAF6CF0F996E4A7710B33CA873C6DEC180A'),
			'type': 'namespace_registration_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '8B13C37938D786D97CC374CB8D35279290210684AD3E9D664FD5F40B7096F2C8',
			'fee': 0x00000000066FF300,
			'rental_fee_sink': 'TCTXZSZTZGQBWS7MRXTWMOR72LPX2YEVKIXGDIHL',
			'rental_fee': 0x0000000028697580,
			# TODO: this is probably wrong in generator, explicit none is required, cause by default it's initialized to empty bytes array
			'parent_name': None,
			'name': b'state',
		},
		'NamespaceRegistrationTransaction_2': {
			'type': 'namespace_registration_transaction',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '8891094EB5092EAFA58A746D86F8666264CAA86E9E94030C71898374AAAFE37D',
			'signature': nc.Signature(
				'EE84DFCC0D0613B3217F6E0C8D68F2B559859F792BFCCA340CFBD173F612A933'
				'A0B0C2FEE9E196CE8332CFAB4CB9D1FD2C445E214B7BD433695CB9125236A001'),
			'fee': 0x00000000066FF300,
			'rental_fee_sink': 'TATVJXFNG7BL7NDHNEQNANEOO2UWA55JPJ7U5I2P',
			'rental_fee': 0x0000000028697580,
			'name': b'controlled',
			'parent_name': b'state',
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
