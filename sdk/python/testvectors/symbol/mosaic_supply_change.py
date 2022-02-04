SAMPLE_ADDRESS = 'SCBQEX7TVCVVVUIEMMP3G4HSSAAESUWND7O4JSI'


def _generate_normal(factory):
	descriptors = {
		'MosaicSupplyChangeTransaction_1': {
			'version': 0x1,
			'type': 'mosaic_supply_change_transaction',
			'deadline': 1,
			'mosaic_id': 0x57701A9B6E746988,
			'action': 'increase',
			'delta': 0xA
		},
		'MosaicSupplyChangeTransaction_3': {
			'version': 0x1,
			'type': 'mosaic_supply_change_transaction',
			'deadline': 0x000000000000022B,
			'mosaic_id': 0xCAF5DD1286D7CC4C,
			'action': 'increase',
			'delta': 0xA
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': 'CC3E835D179670514BE008521130441316AFD4FB09050377B850C61271599405',
			'version': 0x1,
			'type': 'mosaic_supply_change_transaction',
			'mosaic_id': 0x57701A9B6E746988,
			'action': 'increase',
			'delta': 0xA
		})
	]

	return {
		'AggregateBondedTransaction_15': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'D0A68EB4096C1589E531F5B11091ADB71A4D93DE23C7A52A97FE49CDEF35C1E8',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
