from binascii import unhexlify

SAMPLE_ADDRESS = 'SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZQ'


def _generate_normal(factory):
	descriptors = {
		# id is autofilled, so can't really match this transaction
		'NamespaceRegistrationTransaction_1': {
			'type': 'namespace_registration_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'duration': 0x0000000000002710,
			# parent_id: None,
			'id': 0xC053DFAFB8B3E97E,
			'registration_type': 'root',
			'name': unhexlify('6E65776E616D657370616365')

		},
		'NamespaceRegistrationTransaction_3': {
			'type': 'namespace_registration_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'duration': 0x0000000000002710,
			'parent_id': 0x4053DFAFB8B3E97E,
			'id': 0xF1A379781B981203,
			'registration_type': 'child',
			'name': unhexlify('7375626E616D657370616365')
		},
		'NamespaceRegistrationTransaction_5': {
			'type': 'namespace_registration_transaction',
			'deadline': 0x000000000000022B,
			'duration': 0x00000000000003E8,
			# 'parent_id': None,
			'id': 0x9BE64B992DE7CBCF,
			'registration_type': 'root',
			'name': unhexlify('726F6F742D746573742D6E616D657370616365')
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions_21 = [
		factory.create_embedded({
			'type': 'namespace_registration_transaction',
			'signer_public_key': '1E6132AA03A256A450AD0A12BC8A0560584FCBD79BEC86AA502521E5EEA35DF6',
			'version': 1,
			'duration': 0x0000000000002710,
			# parent_id: None,
			'id': 0xC053DFAFB8B3E97E,
			'registration_type': 'root',
			'name': unhexlify('6E65776E616D657370616365')
		})
	]
	embedded_transactions_22 = [
		factory.create_embedded({
			'type': 'namespace_registration_transaction',
			'signer_public_key': 'E940F3DBCD87D52455691DB6E54F1ACDE7030DC2B6895A7562C4CF41BE2A8B5B',
			'version': 1,
			'duration': 0x0000000000002710,
			'parent_id': 0x4053DFAFB8B3E97E,
			'id': 0xF1A379781B981203,
			'registration_type': 'child',
			'name': unhexlify('7375626E616D657370616365')
		})
	]

	return {
		'AggregateBondedTransaction_21': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '0B98E5B3791D95C41B943E480FD744480DB058CE5C1E80F57BCFA32D5CD09BC7',
			'transactions': embedded_transactions_21,
		}),
		'AggregateBondedTransaction_22': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '0897D255849178FAFB1CA484FBFA9AAB023EE41880B445B4F598A207299022B6',
			'transactions': embedded_transactions_22,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
