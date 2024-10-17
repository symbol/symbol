SCHEMA_NAME = 'AccountKeyLinkTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_key_link_transaction_v1',
			'link_action': 'link',
			'remote_public_key': '6269E26026CECEFE640C3E0DE050CB9B3CFD279A0713CF00E16EDEF5D6C10EB9',
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_key_link_transaction_v1',
			'link_action': 'unlink',
			'remote_public_key': 'C3D4EAEB517BDDF22F21A2F7B61194D50666EDEBFE81B9118DB59ABE1D6E98E5',
		}
	}
]
