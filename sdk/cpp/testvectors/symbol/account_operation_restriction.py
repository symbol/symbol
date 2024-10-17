SCHEMA_NAME = 'AccountOperationRestrictionTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_operation_restriction_transaction_v1',
			'restriction_flags': 'outgoing transaction_type',
			'restriction_additions': ['secret_proof'],
			'restriction_deletions': ['transfer']
		}
	},
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'account_operation_restriction_transaction_v1',
			'restriction_flags': 'outgoing transaction_type block',
			'restriction_additions': ['address_alias'],
			'restriction_deletions': []
		}
	}
]
