from binascii import unhexlify

SAMPLE_ADDRESS = 'SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZQ'


def _generate_normal(factory):
	descriptors = {
		'SecretProofTransaction_1': {
			'type': 'secret_proof_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256',
			'proof': unhexlify('9A493664')
		},
		'SecretProofTransaction_3': {
			'type': 'secret_proof_transaction',
			'deadline': 0x000000000000022B,
			'recipient_address': 'QDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WGU4AXA',
			'secret': '9B3155B37159DA50AA52D5967C509B410F5A36A3B1E31ECB5AC76675D79B4A5E',
			'hash_algorithm': 'sha3_256',
			'proof': unhexlify('B778A39A3663719DFC5E48C9D78431B1E45C2AF9DF538782BF199C189DABEAC7')
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': 'DE9B1F9A22014D3E7BC0B336B6D397325D5ABF24A9100B00282C11DD1160730D',
			'version': 0x1,
			'type': 'secret_proof_transaction',
			'recipient_address': SAMPLE_ADDRESS,
			'secret': '3FC8BA10229AB5778D05D9C4B7F56676A88BF9295C185ACFC0F961DB5408CAFE',
			'hash_algorithm': 'sha3_256',
			'proof': unhexlify('9A493664')
		})
	]

	return {
		'AggregateBondedTransaction_19': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '9E82B0F493ABF3FA0C8635C0849025A980EEB66236FF81B6220814D06424AF7F',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
