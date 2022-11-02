SCHEMA_NAME = 'AccountAddressRestrictionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_address_restriction_transaction_v1',
			'restriction_flags': 'address',
			'restriction_additions': ['TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ', 'TD2ASJ2LKL5LX66PPZ67PYQN4HIMH5SX7OCZLQI'],
			'restriction_deletions': ['TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I']
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_address_restriction_transaction_v1',
			'restriction_flags': 'address outgoing block',
			'restriction_additions': ['TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I'],
			'restriction_deletions': []
		}
	}
]
