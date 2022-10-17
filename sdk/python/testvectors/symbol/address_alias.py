SAMPLE_ADDRESS = 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'


recipes = {
	'schema_name': 'AddressAliasTransactionV1',
	'descriptors': [
		{
			'type': 'address_alias_transaction_v1',
			'namespace_id': 0x84B3552D375FFA4B,
			'address': SAMPLE_ADDRESS,
			'alias_action': 'link'
		},
		{
			'type': 'address_alias_transaction_v1',
			'namespace_id': 0x84B3552D375FFA4B,
			'address': SAMPLE_ADDRESS,
			'alias_action': 'unlink'
		}
	]
}
