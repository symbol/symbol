from binascii import unhexlify

SAMPLE_ADDRESS = 'SCBQEX7TVCVVVUIEMMP3G4HSSAAESUWND7O4JSI'


def _generate_normal(factory):
	descriptors = {
		'MosaicMetadataTransaction_1': {
			'type': 'mosaic_metadata_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'target_mosaic_id': 0x00000000000003E8,
			'value_size_delta': 0xA,
			'value': unhexlify('313233414243')
		},
		'MosaicMetadataTransaction_3': {
			'type': 'mosaic_metadata_transaction',
			'deadline': 0x000000000000022B,
			'target_address': 'QDLGYM2CBZKBDGK3VT6KFMUM6HE7LXL2WGU4AXA',
			'scoped_metadata_key': 0x3E8,
			'target_mosaic_id': 0xCAF5DD1286D7CC4C,
			'value_size_delta': 0x1,
			'value': unhexlify('00000000000000000000')
		},
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': '4871937A9B3872130EB65765BF5E0AE326C49ABB39A3F353711AE782D95FF2CB',
			'version': 0x1,
			'type': 'mosaic_metadata_transaction',
			'target_address': SAMPLE_ADDRESS,
			'scoped_metadata_key': 0xA,
			'target_mosaic_id': 0x00000000000003E8,
			'value_size_delta': 0xA,
			'value': unhexlify('313233414243')
		})
	]

	return {
		'AggregateBondedTransaction_1': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '97EE38BB7E04C0C915F3B69B5D6CF77E04B893A86090E417A42660A073515E9C',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
