def _generate_normal(factory):
	descriptors = {
		'AccountOperationRestrictionTransaction_1': {
			'type': 'account_operation_restriction_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'restriction_flags': 'outgoing transaction_type',
			'restriction_additions': ['secret_proof'],
			'restriction_deletions': ['transfer']
		},
		'AccountOperationRestrictionTransaction_3': {
			'type': 'account_operation_restriction_transaction',
			'deadline': 0x000000000000022B,
			'restriction_flags': 'outgoing transaction_type',
			'restriction_additions': ['address_alias'],
			'restriction_deletions': []
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '10DE38DE7D9DB4B9DB52BC61191A6EF5F835DF56459E25E279C72194E03B1F37',
			'version': 0x1,
			'type': 'account_operation_restriction_transaction',
			'restriction_flags': 'outgoing transaction_type',
			'restriction_additions': ['secret_proof'],
			'restriction_deletions': ['transfer']
		})
	]

	return {
		'AggregateBondedTransaction_13': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '97B9CB6B4A5C7EF2B809322AD34DE85BB6CBB2F1A6A2265A77CE3C5276E9CDAE',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
