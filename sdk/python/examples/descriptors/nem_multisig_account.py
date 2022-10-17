def descriptor_factory():
	return [
		# V2 add
		{
			'type': 'multisig_account_modification_transaction_v2',
			'min_approval_delta': 1,
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
					}
				},
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': '677035391CD3701293D385F037BA32796252BB7CE180B00B582DD9B20AAAD7F0'
					}
				}
			]
		},

		# V1 add
		{
			'type': 'multisig_account_modification_transaction_v1',
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
					}
				}
			]
		},

		# V1 del
		{
			'type': 'multisig_account_modification_transaction_v1',
			'modifications': [
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
					}
				}

			]
		}
	]
