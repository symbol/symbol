SAMPLE_ADDRESS = 'SDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WEQE5JA'


def _generate_normal(factory):
	descriptors = {
		'MosaicAddressRestrictionTransaction_1': {
			'type': 'mosaic_address_restriction_transaction',
			'signer_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'mosaic_id': 0x0000000000000001,
			'restriction_key': 0x1,
			'previous_restriction_value': 0x9,
			'new_restriction_value': 0x8,
			'target_address': SAMPLE_ADDRESS
		},
		'MosaicAddressRestrictionTransaction_3': {
			'type': 'mosaic_address_restriction_transaction',
			'deadline': 0x000000000000022B,
			'mosaic_id': 0x0000000000000001,
			'restriction_key': 0x115C,
			'previous_restriction_value': 0x0,
			'new_restriction_value': 0x0,
			'target_address': 'QDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WGU4AXA'
		},
		'MosaicAddressRestrictionTransaction_4': {
			'type': 'mosaic_address_restriction_transaction',
			'deadline': 0x000000000000022B,
			'mosaic_id': 0xD401054C1965C26E,
			'restriction_key': 0x115C,
			'previous_restriction_value': 0x0,
			'new_restriction_value': 0x0,
			'target_address': 'QDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WGU4AXA'
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '2A5029790873C060C001F01FDEC0FC51D9DFE6501A14D28CD2EAC5E711E0CB17',
			'version': 0x1,
			'type': 'mosaic_address_restriction_transaction',
			'mosaic_id': 0x0000000000000001,
			'restriction_key': 0x1,
			'previous_restriction_value': 0x9,
			'new_restriction_value': 0x8,
			'target_address': SAMPLE_ADDRESS
		})
	]

	return {
		'AggregateBondedTransaction_18': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'FCF315E8CB3EC984365EB7AB7B3CED8816E62C0F1E329C01D80919590CB21743',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
