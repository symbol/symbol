recipes = {
	'schema_name': 'MultisigAccountModificationTransactionV1',
	'descriptors': [
		# comment: v1, single add
		{
			'type': 'multisig_account_modification_transaction_v1',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'D27EAF5346C5E01C3EA1EAC431833765167E3C3FB976D436C578BFFC44E44637'
					}
				}
			]
		},
		# comment: v1, add, add, delete
		{
			'type': 'multisig_account_modification_transaction_v1',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'D2FF731FE0CECBA4948173DCFC1CA8860EAABC7CA8F8495753E2AA38562AECB9'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'E401FF49AA42CC10C6FD2227A81C4C78D156B0AFC8B130D7B056784511E6D99E'
					}
				},
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': '50513870D3F5C0977E76B164C6EB0564939FB7C60F7B21F670A7F3234A88F316'
					}
				}
			]
		},
		# comment: v1, add, add, delete (out of order)
		# sorting by (modification_type, ripemd_keccak_256(cosignatory_public_key))
		{
			'type': 'multisig_account_modification_transaction_v1',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'E401FF49AA42CC10C6FD2227A81C4C78D156B0AFC8B130D7B056784511E6D99E'
						# ripemd_keccak_256(cosignatory_public_key): E22B88EB48B0A39B7A070ED6ABC87B519E5B03EF
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'D2FF731FE0CECBA4948173DCFC1CA8860EAABC7CA8F8495753E2AA38562AECB9'
						# ripemd_keccak_256(cosignatory_public_key): B2971B6F06F947AB0FE239DA659589B3A91F8E68
					}
				},
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': '50513870D3F5C0977E76B164C6EB0564939FB7C60F7B21F670A7F3234A88F316'
					}
				}
			]
		}
	]
}
