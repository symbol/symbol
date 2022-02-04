def _generate_normal(factory):
	descriptors = {
		'HashLockTransaction_1': {
			'type': 'hash_lock_transaction',
			'signer_public_key': '2134E47AEE6F2392A5B3D1238CD7714EABEB739361B7CCF24BAE127F10DF17F2',
			'version': 1,
			'deadline': 0x0000000000000001,
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'hash': '8498B38D89C1DC8A448EA5824938FF828926CD9F7747B1844B59B4B6807E878B'
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': 'CE4046D3C66C81725042A1C54A584A00DC171F065EA75603B8799A273735DA72',
			'version': 0x1,
			'type': 'hash_lock_transaction',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'hash': '8498B38D89C1DC8A448EA5824938FF828926CD9F7747B1844B59B4B6807E878B'
		})
	]

	return {
		'AggregateBondedTransaction_10': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '9A5B5EAFD005A7A45417F12E70BD568E510597BF23C54174CB4D9F5D9FF3165D',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
