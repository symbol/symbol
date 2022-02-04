from symbolchain import nc


def generate(factory):
	descriptors = {
		'MultisigAccountModificationTransaction_1': {
			'signature': nc.Signature(
				'A4934619EEE256A55988106A567E68DC0722D08A2962F49D33D795559EAD62C2'
				'3D3B6023B22479214CF95492F662FCDBAC21DA2D8C74AF0BEE5C20101EB01307'),
			'type': 'multisig_account_modification_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '3B882A781F9C314B4FB21106F61A287024899EF092832E3587E27B8606F6269E',
			'fee': 0x0000000000F42400,
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '812C52FD863E43A9CD00A7B7D59D6C6B943784ED98FA21010ACA562A7053FC66'
					}
				}
			]
		},

		'MultisigAccountModificationTransaction_2': {
			'signature': nc.Signature(
				'D3BAD87C40654B11331024414A582D6F1A97D422E094B0210424543B3869C9CC'
				'D33FC35DAA31A3619758FA0443D850DFA849E88B6615A3A6383A7DA339FEAB0D'),
			'type': 'multisig_account_modification_transaction_v1',
			'version': 0x1,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '3B882A781F9C314B4FB21106F61A287024899EF092832E3587E27B8606F6269E',
			'fee': 0x0000000001AB3F00,
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '812C52FD863E43A9CD00A7B7D59D6C6B943784ED98FA21010ACA562A7053FC66'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'A9720C95E3138081E06DE7BD167DA9E8B8B9111E94E31D67974C978E4F694D4D'
					}
				},
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': '02441267EB966BE4C491284F76F2DB6DF6A24AEDF1F611ACF79B253583D8B58B'
					}
				}
			]
		},
		'MultisigAccountModificationTransaction_3': {
			'signature': nc.Signature(
				'776E27D423529FE8514751C170F453B0A3E24FB654E54759EDF3F543C8890442'
				'EBCFECDC8754B8837D210C327FFEABC92284C369CDECFFD96901A7BC10454A00'),
			'type': 'multisig_account_modification_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '3B882A781F9C314B4FB21106F61A287024899EF092832E3587E27B8606F6269E',
			'fee': 0x0000000000F42400,
			'modifications': [],
			'min_approval_delta': 0x2
		},
		'MultisigAccountModificationTransaction_4': {
			'signature': nc.Signature(
				'49CB2565FC297AD4B69F0D09BE0CE17C9738A86EF1B823D64532FADF7E54206A'
				'CCCD2F89FA038F52861F5C4A49F1457C795E7D292C65BCE315FF90ED8B55AA02'),
			'type': 'multisig_account_modification_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '3B882A781F9C314B4FB21106F61A287024899EF092832E3587E27B8606F6269E',
			'fee': 0x00000000014FB180,
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '812C52FD863E43A9CD00A7B7D59D6C6B943784ED98FA21010ACA562A7053FC66'
					}
				},
			],
			# note: signed int
			'min_approval_delta': -1,
		},
		'MultisigAccountModificationTransaction_5': {
			'signature': nc.Signature(
				'5BEE300B8D9A56820CB908282DB7DEF53A1729A52DC3C6FA82F0C0828FF12881'
				'3CF48D1A4C5F0ED3A9300E47C7F2B2AD72AD2F60303FB493B22CE26A68D8AD09'),
			'type': 'multisig_account_modification_transaction',
			'version': 0x2,
			'network': 'testnet',
			'timestamp': 0x0001E240,
			'signer_public_key': '3B882A781F9C314B4FB21106F61A287024899EF092832E3587E27B8606F6269E',
			'fee': 0x000000000206CC80,
			'modifications': [
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': '812C52FD863E43A9CD00A7B7D59D6C6B943784ED98FA21010ACA562A7053FC66'
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': 'A9720C95E3138081E06DE7BD167DA9E8B8B9111E94E31D67974C978E4F694D4D'
					}
				},
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': '02441267EB966BE4C491284F76F2DB6DF6A24AEDF1F611ACF79B253583D8B58B'
					}
				}
			],
			'min_approval_delta': 0x1
		}
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}
