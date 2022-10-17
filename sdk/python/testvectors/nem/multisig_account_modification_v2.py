recipes = {
	'schema_name': 'MultisigAccountModificationTransactionV2',
	'descriptors': [
		# comment: v2, no-op, increase min-approval
		{
			'type': 'multisig_account_modification_transaction_v2',
			'modifications': [],
			'min_approval_delta': 0x2
		},
		# comment: v2, add, decrease min-approval
		{
			'type': 'multisig_account_modification_transaction_v2',
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
			'type': 'multisig_account_modification_transaction_v2',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'D79936328C188A4416224ABABF580CA2C5C8D852248DB1933FE4BC0DCA0EE7BC'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '5D378657691CAD70CE35A46FB88CB134232B0B6B3655449C019A1F5F20AE9AAD'
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
		},
		# comment: v2, add, add, del, increase min-approval (out of order)
		# sorting by (modification_type, ripemd_keccak_256(cosignatory_public_key))
		{
			'type': 'multisig_account_modification_transaction_v2',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '5D378657691CAD70CE35A46FB88CB134232B0B6B3655449C019A1F5F20AE9AAD'
						# ripemd_keccak_256(cosignatory_public_key): 157D0CC1E0587A3EC374E10A8AE1B7681BFF3384
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'D79936328C188A4416224ABABF580CA2C5C8D852248DB1933FE4BC0DCA0EE7BC'
						# ripemd_keccak_256(cosignatory_public_key): 10BA1653D2A0BCE89D4EBC7C4B02842431C276CD
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
