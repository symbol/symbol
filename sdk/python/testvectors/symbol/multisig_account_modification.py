SAMPLE_ADDRESS_1 = 'SBPNENBVQLP3JUKNZA336GHDZG7FE4P7TOFJ5QI'
SAMPLE_ADDRESS_2 = 'SCDWANU5Y6DWDZ7LZRWPV2SE52KG5UDDPNT64VI'


def _generate_normal(factory):
	descriptors = {
		'MultisigAccountModificationTransaction_1': {
			'type': 'multisig_account_modification_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'min_removal_delta': 0x1,
			'min_approval_delta': 0x2,
			'address_additions': [SAMPLE_ADDRESS_1],
			'address_deletions': [SAMPLE_ADDRESS_2]
		},
		'MultisigAccountModificationTransaction_3': {
			'type': 'multisig_account_modification_transaction',
			'deadline': 0x000000000000022B,
			'min_removal_delta': 0x1,
			'min_approval_delta': 0x2,
			'address_additions': ['QCP4JBCKKIDM7JCGAPX2D76HN7U3AVSNSZZVKYQ'],
			'address_deletions': []
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '3ACBD9C4989E7DDFD0AAB3E3CD014DF6A8F301E6FF63874A50981DF75EEF86DC',
			'version': 0x1,
			'type': 'multisig_account_modification_transaction',
			'min_removal_delta': 0x1,
			'min_approval_delta': 0x2,
			'address_additions': [SAMPLE_ADDRESS_1],
			'address_deletions': [SAMPLE_ADDRESS_2]
		})
	]

	return {
		'AggregateBondedTransaction_12': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'A7197AB28F6EACC591B5D16D77BA56160BDA7827B159C0586DF6152802C9CFEB',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
