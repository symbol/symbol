from binascii import unhexlify

SAMPLE_ADDRESS = 'SCBQEX7TVCVVVUIEMMP3G4HSSAAESUWND7O4JSI'


def _generate_normal(factory):
	descriptors = {
		'NamespaceMetadataTransaction_1': {
			'type': 'namespace_metadata_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'target_namespace_id': 0x00000000000003E8,
			'value_size_delta': 0xA,
			'value': 'ABC123'
		},
		'NamespaceMetadataTransaction_3': {
			'type': 'namespace_metadata_transaction',
			'deadline': 0x000000000000022B,
			'target_address': 'QDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WGU4AXA',
			'scoped_metadata_key': 0x3E8,
			'target_namespace_id': 0xCAF5DD1286D7CC4C,
			'value_size_delta': 0x1,
			'value': unhexlify('00000000000000000000')
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '5E541F8B249C363CDFA0188C88A1E26DB75F30A366A05AA0D6400E9DC41D0257',
			'version': 0x1,
			'type': 'namespace_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'target_namespace_id': 0x00000000000003E8,
			'value_size_delta': 0xA,
			'value': 'ABC123'
		})
	]

	return {
		'AggregateBondedTransaction_14': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'F12AD5DBD30E62E64BCE9404809CBB4F741BE711520F3FB17764760E91570EE2',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
