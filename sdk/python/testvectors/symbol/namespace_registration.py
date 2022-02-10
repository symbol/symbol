recipes = {
	'schema_name': 'NamespaceRegistrationTransaction',
	'descriptors': [
		{
			'type': 'namespace_registration_transaction',
			'duration': 0x0000000000002710,
			# parent_id: None,
			'id': 0xC053DFAFB8B3E97E,
			'registration_type': 'root',
			'name': 'newnamespace'
		},
		{
			'type': 'namespace_registration_transaction',
			'parent_id': 0x4053DFAFB8B3E97E,
			'id': 0xF1A379781B981203,
			'registration_type': 'child',
			'name': 'subnamespace'
		}
	]
}
