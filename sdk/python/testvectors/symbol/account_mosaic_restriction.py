SAMPLE_ADDRESS = 'SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZQ'


def _generate_normal(factory):
	descriptors = {
		'AccountMosaicRestrictionTransaction_1': {
			'type': 'account_mosaic_restriction_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'restriction_flags': 'mosaic_id',
			'restriction_additions': [0x00000000000003E8],
			'restriction_deletions': [0x00000000000007D0]
		},
		'AccountMosaicRestrictionTransaction_3': {
			'type': 'account_mosaic_restriction_transaction',
			'version': 0x1,
			'network': 'testnet',
			'deadline': 0x000000000000022B,
			'restriction_flags': 'mosaic_id',
			'restriction_additions': [0xCAF5DD1286D7CC4C],
			'restriction_deletions': []
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'type': 'account_mosaic_restriction_transaction',
			'signer_public_key': '16ED504C968BAD1F09EBE902E91CF8D7660381CFC10214FB2D141E8DAEDC91B4',
			'version': 0x1,
			'restriction_flags': 'mosaic_id',
			'restriction_additions': [0x00000000000003E8],
			'restriction_deletions': [0x00000000000007D0]
		})
	]

	return {
		'AggregateBondedTransaction_28': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'A5D2296871C6DCEAD69D52F96110BC22D26F4E2DE963DCF268AEF848C3366822',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
