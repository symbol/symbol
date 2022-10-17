export default () => ([
	// V2 add
	{
		type: 'multisig_account_modification_transaction_v2',
		minApprovalDelta: 1,
		modifications: [
			{
				modification: {
					modificationType: 'add_cosignatory',
					cosignatoryPublicKey: '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF'
				}
			},
			{
				modification: {
					modificationType: 'add_cosignatory',
					cosignatoryPublicKey: 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
				}
			},
			{
				modification: {
					modificationType: 'delete_cosignatory',
					cosignatoryPublicKey: '677035391CD3701293D385F037BA32796252BB7CE180B00B582DD9B20AAAD7F0'
				}
			}
		]
	},

	// V1 add
	{
		type: 'multisig_account_modification_transaction_v1',
		modifications: [
			{
				modification: {
					modificationType: 'add_cosignatory',
					cosignatoryPublicKey: '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF'
				}
			},
			{
				modification: {
					modificationType: 'add_cosignatory',
					cosignatoryPublicKey: 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
				}
			}
		]
	},

	// V1 del
	{
		type: 'multisig_account_modification_transaction_v1',
		modifications: [
			{
				modification: {
					modificationType: 'delete_cosignatory',
					cosignatoryPublicKey: 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
				}
			}

		]
	}
]);
