def _generate_normal(factory):
	descriptors = {
		'MosaicGlobalRestrictionTransaction_1': {
			'type': 'mosaic_global_restriction_transaction',
			'signer_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'mosaic_id': 0x0000000000000D80,
			'reference_mosaic_id': 0x0000000000000002,
			'restriction_key': 0x1,
			'previous_restriction_value': 0x9,
			'new_restriction_value': 0x8,
			'previous_restriction_type': 'eq',
			'new_restriction_type': 'ge'

		},
		'MosaicGlobalRestrictionTransaction_3': {
			'type': 'mosaic_global_restriction_transaction',
			'deadline': 0x000000000000022B,
			'mosaic_id': 0x0000000000000001,
			'reference_mosaic_id': 0x0000000000000000,
			'restriction_key': 0x115C,
			'previous_restriction_value': 0x0,
			'new_restriction_value': 0x0,
			'previous_restriction_type': 'none',
			'new_restriction_type': 'ge'
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'type': 'mosaic_global_restriction_transaction',
			'signer_public_key': '687FE391241D67448B0E02C5D34E09A4EF880509EF8C5B42570749F012AA09B4',
			'version': 0x1,
			'mosaic_id': 0x0000000000000D80,
			'reference_mosaic_id': 0x0000000000000002,
			'restriction_key': 0x1,
			'previous_restriction_value': 0x9,
			'new_restriction_value': 0x8,
			'previous_restriction_type': 'eq',
			'new_restriction_type': 'ge'
		})
	]

	return {
		'AggregateBondedTransaction_27': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'EAB01ABB1677368FAC2F6BF706A28ED8B659F40ECD0D1BD3BC718D9BF83108CD',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
