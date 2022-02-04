from symbolchain import sc

SAMPLE_ADDRESS = 'SBE6CS7LZKJXLDVTNAC3VZ3AUVZDTF3PACNFIXA'


def _generate_normal(factory):
	descriptors = {
		'AddressAliasTransaction_1': {
			'type': 'address_alias_transaction',
			'signature': sc.Signature(
				'164DC06341FE6FAC16EF51663F04113049B5CC3B648043EDE1D8BBF4BF16B7B8'
				'933F7E42A30B84A6D1EAB5CCECD8E4462923323E5816BED2134D54013B937D1A'),
			'signer_public_key': '68B3FBB18729C1FDE225C57F8CE080FA828F0067E451A3FD81FA628842B0B763',
			'version': 1,
			'fee': 0x0000000000000001,
			'deadline': 0x0000000000000001,
			'namespace_id': 0x84B3552D375FFA4B,
			'address': SAMPLE_ADDRESS,
			'alias_action': 'link'
		},
		'AddressAliasTransaction_3': {
			'type': 'address_alias_transaction',
			'deadline': 0x000000000000022B,
			'namespace_id': 0xE1499A8D01FCD82A,
			'address': 'QATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA367I6OQ',
			'alias_action': 'link'
		},
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': 'ACBD55170BE49239EC69279A3921578B5701CA7AF4A1A5FF87B2B2957120AC87',
			'version': 0x1,
			'type': 'address_alias_transaction',
			'namespace_id': 0x84B3552D375FFA4B,
			'address': SAMPLE_ADDRESS,
			'alias_action': 'link'
		})
	]

	return {
		'AggregateBondedTransaction_3': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '6A155E534B494F0CC34AFBB78371B1446EAD405FFFE4B263AA05A43D3167C5D9',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
