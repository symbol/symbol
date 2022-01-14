from symbolchain.symbol.IdGenerator import generate_namespace_id


def descriptor_factory():
	return [
		{
			'type': 'namespace_registration_transaction',
			'registration_type': 'root',
			'duration': 123,
			'name': 'roger'
		},

		{
			'type': 'namespace_registration_transaction',
			'registration_type': 'child',
			'parent_id': generate_namespace_id('roger'),
			'name': 'charlie'
		}
	]
