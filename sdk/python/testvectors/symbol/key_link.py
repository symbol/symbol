def _generate_normal(factory):
	descriptors = {
		'AccountKeyLinkTransaction_1': {
			'type': 'account_key_link_transaction',
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 1,
			'deadline': 0x0000000000000001,
			'linked_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'link_action': 'link'
		},
		'AccountKeyLinkTransaction_3': {
			'type': 'account_key_link_transaction',
			'deadline': 0x000000000000022B,
			'linked_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'link_action': 'link'
		},

		'VrfKeyLinkTransaction_1': {
			'type': 'vrf_key_link_transaction',
			'deadline': 0x000000000000022B,
			'linked_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'link_action': 'link'
		},
		'NodeKeyLinkTransaction_1': {
			'type': 'node_key_link_transaction',
			'deadline': 0x000000000000022B,
			'linked_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'link_action': 'link'
		},
		'VotingKeyLinkTransaction_1': {
			'type': 'voting_key_link_transaction',
			'deadline': 0x000000000000022B,
			'linked_public_key': 'C614558647D02037384A2FECA80ACE95B235D9B9D90035FA46102FE79ECCBA75',
			'start_epoch': 0x00000001,
			'end_epoch': 0x00000003,
			'link_action': 'link'
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions = [
		factory.create_embedded({
			'signer_public_key': 'AE4A52DDFF64ECDEB4B017228B5411DDFFB1474ED7B7C7D2671C71B8D789F3EB',
			'version': 0x1,
			'type': 'account_key_link_transaction',
			'linked_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'link_action': 'link'
		})
	]

	return {
		'AggregateBondedTransaction_11': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '584F02318224C48C34CB4E321F293EA6BA2A12EF68E82713003F0AC31A8AD34A',
			'transactions': embedded_transactions,
		})
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
