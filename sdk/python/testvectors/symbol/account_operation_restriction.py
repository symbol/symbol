recipes = {
	'schema_name': 'AccountOperationRestrictionTransactionV1',
	'descriptors': [
		{
			'type': 'account_operation_restriction_transaction_v1',
			'restriction_flags': 'outgoing transaction_type',
			'restriction_additions': ['secret_proof'],
			'restriction_deletions': ['transfer']
		},
		{
			'type': 'account_operation_restriction_transaction_v1',
			'restriction_flags': 'outgoing transaction_type block',
			'restriction_additions': ['address_alias'],
			'restriction_deletions': []
		}
	]
}
