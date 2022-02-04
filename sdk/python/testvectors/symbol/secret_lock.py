SAMPLE_ADDRESS = 'SDZWZJUAYNOWGBTCUDBY3SE5JF4NCC2RDM6SIGQ'


def _generate_normal(factory):
	descriptors = {
		'SecretLockTransaction_1': {
			'type': 'secret_lock_transaction',
			'signer_public_key': '9A49366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456B24',
			'version': 1,
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'hash_algorithm': 'sha3_256'
		},
		'SecretLockTransaction_3': {
			'type': 'secret_lock_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256'
		},
		'SecretLockTransaction_5': {
			'type': 'secret_lock_transaction',
			'deadline': 0x000000000000022B,
			'recipient_address': 'QCOXVZMAZJTT4I3F7EAZYGNGR77D6WPTREIM2RQ',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x000000000000000A},
			'duration': 100,
			'secret': '9B3155B37159DA50AA52D5967C509B410F5A36A3B1E31ECB5AC76675D79B4A5E',
			'hash_algorithm': 'sha3_256'
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions_16 = [
		factory.create_embedded({
			'signer_public_key': '83703D89B194758D2119B4A6EE56C2544120C9FA6CADE47103AE7FA7875570C0',
			'version': 0x1,
			'type': 'secret_lock_transaction',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256'
		})
	]

	embedded_transactions_17 = [
		factory.create_embedded({
			'signer_public_key': '54DE12E65453D160A3F52794BC32789AC4A3E97ADD0F7967ADF84B0D90738BBA',
			'version': 0x1,
			'type': 'secret_lock_transaction',
			'recipient_address': SAMPLE_ADDRESS,
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680},
			'duration': 100,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256'
		})
	]

	return {
		'AggregateBondedTransaction_16': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'DDF18516A2274AB73210FEF6D7AFA32212678E709475986BD067ED688D6E4B85',
			'transactions': embedded_transactions_16,
		}),

		'AggregateBondedTransaction_17': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '0A05CEB203C6BAC9DB1548077121C7F7A885AFDC463F300DD0976653570FD5BB',
			'transactions': embedded_transactions_17,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
