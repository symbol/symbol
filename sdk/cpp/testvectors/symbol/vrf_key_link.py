SCHEMA_NAME = 'VrfKeyLinkTransactionV1'


transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'vrf_key_link_transaction_v1',
			'linked_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'link_action': 'link'
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'vrf_key_link_transaction_v1',
			'linked_public_key': 'C614558647D02037384A2FECA80ACE95B235D9B9D90035FA46102FE79ECCBA75',
			'link_action': 'unlink'
		}
	}
]
