def descriptor_factory():
	return [
		# root namespace
		{
			'type': 'namespace_registration_transaction',
			'rental_fee_sink': 'TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35',
			'rental_fee': 50000 * 1000000,
			'parent_name': '',
			'name': 'roger'
		},

		# child namespace
		{
			'type': 'namespace_registration_transaction',
			'rental_fee_sink': 'TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35',
			'rental_fee': 1 * 1000000,
			'parent_name': 'roger',
			'name': 'charlie'
		}
	]
