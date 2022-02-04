SAMPLE_ADDRESS = 'SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZQ'


def _generate_normal(factory):
	descriptors = {
		# id is autofilled, so can't really match this transaction
		'MosaicDefinitionTransaction_1': {
			'type': 'mosaic_definition_transaction',
			'deadline': 0x0000000000000001,
			'duration': 0x0000000000002710,
			'nonce': 0x00000000,
			'flags': 'restrictable supply_mutable',
			'divisibility': 4
		},
		'MosaicDefinitionTransaction_3': {
			'type': 'mosaic_definition_transaction',
			'deadline': 0x000000000000022B,
			'duration': 0x00000000000003E8,
			'nonce': 0xB884DEE6,
			'flags': 'none',
			'divisibility': 3
		},
		'MosaicDefinitionTransaction_4': {
			'type': 'mosaic_definition_transaction',
			'deadline': 0x000000000000022B,
			'duration': 0x0000000000000000,
			'nonce': 0xB884DEE6,
			'flags': 'none',
			'divisibility': 3
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'type': 'mosaic_definition_transaction',
			'signer_public_key': 'AF906081C12C065AE0DF8AE5822513A5A5E1046D86C272A42A3283E9DFF9C9D3',
			'version': 1,
			'id': 0x0000000000000000,
			'duration': 0x0000000000002710,
			'nonce': 0x00000000,
			'flags': 'restrictable supply_mutable',
			'divisibility': 4
		})
	]

	return {
		'AggregateBondedTransaction_20': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'D7C1075FF2D14C78AD1CB5974BB53370DB3F964BBB0E2DB693E4FDB1F23827F7',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
