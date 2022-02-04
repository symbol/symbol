def _generate_normal(factory):
	descriptors = {
		'MosaicAliasTransaction_1': {
			'type': 'mosaic_alias_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'namespace_id': 0xB6F1FD51147987A4,
			'mosaic_id': 0x000000000000000A,
			'alias_action': 'link'
		},
		'MosaicAliasTransaction_3': {
			'type': 'mosaic_alias_transaction',
			'deadline': 0x000000000000022B,
			'namespace_id': 0xE1499A8D01FCD82A,
			'mosaic_id': 0xCAF5DD1286D7CC4C,
			'alias_action': 'link'
		},
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '4E0184D19FD1D9427BB6888BDDC56756686829A1CAD6686D8D53339E9844CE00',
			'version': 0x1,
			'type': 'mosaic_alias_transaction',
			'namespace_id': 0xB6F1FD51147987A4,
			'mosaic_id': 0x000000000000000A,
			'alias_action': 'link'
		})
	]

	return {
		'AggregateBondedTransaction_2': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '3856BB39656064ADA2971A0791F228069B1F2293DBF1344B687C8E743AEC6B23',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
