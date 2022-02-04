from binascii import unhexlify

SAMPLE_ADDRESS = 'SCBQEX7TVCVVVUIEMMP3G4HSSAAESUWND7O4JSI'


def _generate_normal(factory):
	descriptors = {
		'AccountMetadataTransaction_1': {
			'type': 'account_metadata_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'value_size_delta': 0xA,
			'value': unhexlify('313233424143')
		},
		'AccountMetadataTransaction_3': {
			'type': 'account_metadata_transaction',
			'deadline': 0x000000000000022B,
			'target_address': 'QDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WGU4AXA',
			'scoped_metadata_key': 0x3E8,
			'value_size_delta': 0x1,
			'value': unhexlify('00000000000000000000')
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': 'FA8EC085AE64CF30E44ADD18A3133D9B2190F9A20C08667A5EF44E5E9962E720',
			'version': 0x1,
			'type': 'account_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'value_size_delta': 0xA,
			'value': unhexlify('313233424143')
		})
	]

	return {
		'AggregateBondedTransaction_4': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'AD9E3FFDF99E080F6715FE208A9D96A09B39A3BD7B065AF7F6B11827F1EEE1DF',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
