from symbolchain.core.sym.IdGenerator import generate_namespace_id


def descriptor_factory():
    return [
        {
            'type': 'namespaceRegistration',
            'registration_type': 'root',
            'duration': 123,
            'name': 'roger'
        },

        {
            'type': 'namespaceRegistration',
            'registration_type': 'child',
            'parent_id': generate_namespace_id('roger'),
            'name': 'charlie'
        }
    ]
