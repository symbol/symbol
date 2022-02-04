SAMPLE_ADDRESS_1 = 'SCBQEX7TVCVVVUIEMMP3G4HSSAAESUWND7O4JSI'
SAMPLE_ADDRESS_2 = 'SCZYPI44BZDAPW3QK3XKV4FA55B3IXDGP23ZB7Y'


def _generate_normal(factory):
	descriptors = {
		'AccountAddressRestrictionTransaction_1': {
			'type': 'account_address_restriction_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'restriction_flags': 'address',
			'restriction_additions': [SAMPLE_ADDRESS_1],
			'restriction_deletions': [SAMPLE_ADDRESS_2]
		},
		'AccountAddressRestrictionTransaction_3': {
			'type': 'account_address_restriction_transaction',
			'deadline': 0x000000000000022B,
			'restriction_flags': 'address',
			'restriction_additions': ['QATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA367I6OQ'],
			'restriction_deletions': []
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '5085C164D5D55CD6AF5A4FDABF88D1A2EE2C1C1422D431BAFDC14714ED63E3F5',
			'version': 0x1,
			'type': 'account_address_restriction_transaction',
			'restriction_flags': 'address',
			'restriction_additions': [SAMPLE_ADDRESS_1],
			'restriction_deletions': [SAMPLE_ADDRESS_2]
		})
	]

	return {
		'AggregateBondedTransaction_5': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '4DC6F0524C486D78A6D9D775F5508C0362125420728D03DE74435EB1E3778891',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
