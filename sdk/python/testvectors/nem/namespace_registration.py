recipes = {
	'schema_name': 'NamespaceRegistrationTransaction',
	'descriptors': [
		# comment: empty parent - root ns
		{
			'type': 'namespace_registration_transaction',
			'rental_fee_sink': 'TCTXZSZTZGQBWS7MRXTWMOR72LPX2YEVKIXGDIHL',
			'rental_fee': 0x0000000028697580,
			'parent_name': None,
			'name': b'state',
		},
		# comment: non-empty parent - child ns
		{
			'type': 'namespace_registration_transaction',
			'rental_fee_sink': 'TATVJXFNG7BL7NDHNEQNANEOO2UWA55JPJ7U5I2P',
			'rental_fee': 0x0000000028697580,
			'name': b'controlled',
			'parent_name': b'state',
		}
	]
}
