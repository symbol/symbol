recipes = {
	'schema_name': 'MultisigAccountModificationTransaction',
	'descriptors': [
		# comment: v2, no-op, increase min-approval
		{
			'type': 'multisig_account_modification_transaction',
			'modifications': [],
			'min_approval_delta': 0x2
		},
		# comment: v2, add, decrease min-approval
		{
			'type': 'multisig_account_modification_transaction',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '37D357E29CA8420FA0D615D8D1C4EEBAC06825FAC2FE4A0E28909CF6F79E0DD4'
					}
				},
			],
			# note: signed int
			'min_approval_delta': -1,
		},
		# comment: v2, add, add, del, increase min-approval
		{
			'type': 'multisig_account_modification_transaction',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '5D378657691CAD70CE35A46FB88CB134232B0B6B3655449C019A1F5F20AE9AAD'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'D79936328C188A4416224ABABF580CA2C5C8D852248DB1933FE4BC0DCA0EE7BC'
					}
				},
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': 'BCEAE8AC9F6630893C75381CFA44BA971E556EA245C851D5CB224A18D6871843'
					}
				}
			],
			'min_approval_delta': 0x1
		}
	]
}
