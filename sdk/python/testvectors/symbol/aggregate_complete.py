from binascii import unhexlify

from symbolchain import sc


def _create_cosignature(signer, signature):
	cosignature = sc.Cosignature()
	cosignature.signer_public_key = sc.PublicKey(signer)
	cosignature.signature = sc.Signature(signature)
	return cosignature


def generate(factory):
	return {
		'AggregateCompleteTransaction_1': factory.create({
			'signature': sc.Signature(
				'8C281CF19399A4CD7C97336B73F21D395BF296DB2FF8020A5BFDE51BD3314506'
				'C0C4BE23A625E71DEAE87E20E565B7684D5ECD7C941DF9847691834D19652C08'),
			'signer_public_key': '30EC782177FFEFEE6B8C2B6C38BDFF7413A7872386D4B8A600E255DFD0420903',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '61A53A5CD380F63A506A1059FE2D13FC0DA712E4B39B217407ECECB5DB7DA60D',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'F7847D28C15F11FED0C16401DA9F1D3D67E5BE14DD00521CB293D13CD28F06A1',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SBMWT3XMI2YMY3OZBJERAMDEEW37ZRCY36EE6CY',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765')
				}),
				factory.create_embedded({
					'signer_public_key': 'BAC8F60B0467AFCDA153477D2446921543D3C2BEB5E964F26F9AA62D4FB0A916',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SCYKKZNWNZWRBOAVDLLW67CAIALMIRGIDTM5LPI',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765')
				})
			],
			'cosignatures': [
				_create_cosignature(
					'BAC8F60B0467AFCDA153477D2446921543D3C2BEB5E964F26F9AA62D4FB0A916',
					'677E32A0DA9F62FC71BD5728350EEF38BF4968AA052EAED678DEBD17CF099BF0C3A37C6FE4D585392411418A1892530B423DB7F791D17A44781B10EB398E4605')
			]
		}),
		'AggregateCompleteTransaction_2': factory.create({
			'signature': sc.Signature(
				'D9ADCBC1F31F1370B236510B6A8B184A4A81F4227FFAD2918EDA68827CF36B85'
				'BB6991A8AD6FC0D8CE012F2CAED37393CDCAA10C28A08DB8BD3DDB0CF129260D'),
			'signer_public_key': '1CF46D84415E6C36658D4A96964A0DB51264CC9CE3C916590C5BDD60A94A89D1',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '8857FF8809FDA3AE529CD0EF67EEE479769BBFF0F46847D7AAED856E02EB3911',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '0AA72E6B094163E34DDF9F6210EDA4535426E193D3098B47625BB11B016DA454',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SDQVR2UVPDHJNZUYTKFXA4KCK7CNV3P4QARSV7A',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [
				_create_cosignature(
					'4C2AABC45AAEF79B98A752C94ADA39B0B385C485582A1529C33978371BEE2904',
					'BFBA7ED597EE29300BEDD4CE05C56E67F8D0ACE8B14E26D62CF5B038A144DE96BFB1048B9776DAD3A45977D20ABA34E7A34103B3B174FFD900E3B5092F8C8F01')
			]
		}),
		'AggregateCompleteTransaction_4': factory.create({
			'signature': sc.Signature(
				'2DEFDA9AE95AF71F4A79A478C47C55600F0FD63C5CC99547118467417F8DAA34'
				'E57506E9777E56358043D722BE2DFCA642321D83BE66A539AF31CAA3B395840D'),
			'signer_public_key': '07DB7062C734B5CDEDC1C86D0C6A2688892A8831F724216B5D0E799742F6DCA6',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '484A6E128BA0CCFBE1A8111FEB9341B3BAE4B04B7C220BD45316822C52A00EBC',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'E0B29B19E4FE4CF99155ECF9204A0250DA09F62FDEFDCA6BBD82D1D36046A2D8',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SDKHJYUH65CN3SVWQYFMXSZ57EOWCZBEEEGQM4I',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [],
		}),
		'AggregateCompleteTransaction_5': factory.create({
			'signature': sc.Signature(
				'95F997295188DF9898B0A5E8DADBC78B205FFA0401A6621C0EEDCECD53471C09'
				'CFC626C19D630B0EC3BEC007084B39A5057DC143C20717CF46136BCE8A8B0203'),
			'signer_public_key': '32060636AB9CFDFBDF49DFD6E205C32BF6E5EF880F6ACD6FE5563304010028CC',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '052579A0684C80256E2F7400283C3676545378A6A1472F7B2784A62B4E7D7182',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '32060636AB9CFDFBDF49DFD6E205C32BF6E5EF880F6ACD6FE5563304010028CC',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SCV357SWUISXQDSWL3MYZ6WRIKMTMMPTUQ67NPY',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': []
		}),
		'AggregateCompleteTransaction_6': factory.create({
			'signature': sc.Signature(
				'97235CFDC7ABA2500CF0A349F25EB607046E3CCBBF1A1DB5C6E5CE40426FD686'
				'E64861CF9A412E8254ADF5E2A3AA8BCA3BDBE40E480A0C148CBC1E30F7FC2E0B'),
			'signer_public_key': 'E02E67EF8B71B8D6521C5B3B7CB02431B7A97054DA276B139F9D2F3E3644EF88',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '00419BCE435AC874552B3D0706E6DFC34533FBA60F9BFA281A67A6A4DAF1AC16',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'A52D9ABD9B2DD81E65E3ECE86D8EA2758B185657AEA543EC14C4C82EBC835211',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SDS6BYYB4TYJHCREPOBRPGXVIUT5JDP3MYV3UKY',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				}),
				factory.create_embedded({
					'signer_public_key': '99A38966220E8B3F50466FC126700D5ECF56EEC8C9C65CDB884CE36ADB31585B',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SDS6BYYB4TYJHCREPOBRPGXVIUT5JDP3MYV3UKY',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [
				_create_cosignature(
					'99A38966220E8B3F50466FC126700D5ECF56EEC8C9C65CDB884CE36ADB31585B',
					'C83BA60B6A86EB03FEBA96CCD325ECE600917B2CFFF09432F79C4FB9C7447F4A'
					'A7D352B67C9339F76CF4A4C5EA412C7816114782D87D159CB95E020134C89800'),
				_create_cosignature(
					'7FB15317341AE0053BDBAF72AFD42B275FAE6F442CEBD59590388E348F8510B0',
					'E6CC6741E175700D529446022EA0220E948AA012A182011BC70056702186086C'
					'5EA97E707E75A65FF9F19598262068FD1B1CF8156926F45C95F2FEEC6DAE5700')
			]
		}),
		'AggregateCompleteTransaction_7': factory.create({
			'signature': sc.Signature(
				'3B4528319A3087FFB7FD5F4D8D45FBAE2F5450885AF398A9048A747F3718A97B'
				'31B358623D9465910AD6C8116696DD6E617B85E7AC4729905DCA3E226D394803'),
			'signer_public_key': '51A4458636D21A5DDC33034574E43DE8B7FB9097418B0B41DA24312590355E88',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': 'B1C41CBC5C8FF9B68AF648D90B63D563EDA33DFE3C5638CD3DB5BCED1F78AA96',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '6A5B17934641B25F8A6D44763AD06F951DC9DD8A7E9FDECC424C13B5D7EDB6F4',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SAHON2XX3PCQVLFBSZ5T3AROVW2HOOAQ2BVHTEI',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [],
		}),
		'AggregateCompleteTransaction_9': factory.create({
			'signature': sc.Signature(
				'69AAA1BFC3E1B1507B7D1D2EF68EE62EA02522B86D702F076D8C11B41A7FD086'
				'0DDE5450CD3D4D584DF164CF5AF3A777FD3DBCA1C12C54DB6CF26813FF74CC0D'),
			'signer_public_key': 'F61A834C1C9D4F132AC38CC9AF46E4ABC82D0AB7DAD9280CE9CB69F9EE8AAA23',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': 'FB482B2C78876E91BD8EBDFB58B1E59A949134CB50522F8F8470C5613A420204',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '0B30AFBE29CDD0B8BD191CCAA97EF787F26A5D822A2A1FC95041959017700809',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SD5AG3BC4C6ZSZG6N53SRWFBAFJ6J6OKEIS7QYI',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [
				_create_cosignature(
					'95C68125575891B5F403259E2AAFEC1D7CCA9A750AF462900BEFF31B16351754',
					'A232CA3F026E5FD73B56695C15E710AD66B3120009A2B0FB62CAA72B55F4FBFB'
					'D90598D56D9CC55DD9C97DC54D88BE3F7C117BD5CC2AF9CD4E577CE3062E250C'),
			]
		}),

		# multisig_account_modification out of blue
		'AggregateCompleteTransaction_11': factory.create({
			'signature': sc.Signature(
				'7628D261A89D8281BB5202D55FAD2977D7439059F2F1B971AFF6719236E62367'
				'0DD0EB74F5B3FB8F540E1FA2CB08EA82A3FD637697BD01690875792FC4D8A707'),
			'signer_public_key': 'D4C20FDCB197500B29599940BC84BE2F5DCEF68853F1E7094CAD9F8B84C61CC8',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '83F5ABA0102618F4837F28421819803E42A79B70C89E583D1784FFB7D19DF71D',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '2DF750A9F35247EEDF21AB13264C46B2756E915060E48A21B0C7107A83718873',
					'version': 0x1,
					'network': 'testnet',
					'type': 'multisig_account_modification_transaction',
					'min_removal_delta': 0x1,
					'min_approval_delta': 0x1,
					'address_additions': [],
					'address_deletions': ['SD5QYC5ROK5LF3BQO5VFVEIPZNUK6CP4WFHPDUA']
				}),
			],
			'cosignatures': [],
		}),

		'AggregateCompleteTransaction_12': factory.create({
			'signature': sc.Signature(
				'87FEA49B4308A286A050690C1E079980E69E9E65B1F99E39194317A444D37DFE'
				'82EBC2EB95823D586726CA55B4097EFD1285D38D1E9BBD0632772A8F3B5C980A'),
			'signer_public_key': 'A6827555599AC64F1CF33CEF784A4E04039118971D0DFC39149317E43AE1B810',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '921DDFF1DD6B7358B9C4A52BB4CC9BF0A1E2CC91F6CBCBDEA24B662ABB012A09',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'E822E490EF2A32A81821DCD970AD2C9E3B348BEC246EFAB71E75C72DBCE98AF5',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SDINYAJF6CUHBFVLUO4XQGR655QICNKGUGBKMUQ',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [
				_create_cosignature(
					'D8B2A466CF8967CFA5C9D8A9989FFE7E620E8A84D00696ACFB80FBFABB83FDD2',
					'EF36E6D8B4A339E12E77C857DF7462A319DC41D099F63805A85CD6AF7EE44D45'
					'91F54B3A5B0FFC61BC6DC98BF461CC59FE4F364E425A8D22674151A78FC3C104'),
			],
		}),
		'AggregateCompleteTransaction_13': factory.create({
			'signature': sc.Signature(
				'9EF4DBFD12C05BE89ECE867D36C6498032DE4B5550DA0ACF437D46A14609C737'
				'8A8C4D3B4FBD37DFD2C9FCEC7B3CEB1D33A0AE1409932A06EB199930CB65810F'),
			'signer_public_key': '0675600F69145174A73300C71B35DF199C94B4EAD66E51F83A54F6C78FC2B7A7',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '9E918C130953B0948AE3133039DBFE27FF6FDBB5F0FF2837BB647D329AD3A7ED',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'F6E9D20ED1F24CC2329AE7FC32641D561BEB0E79938F2EEA193CB831D0032083',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SBIAJVVEMLO4RQFKL4HSPJD3DPOISMOT7V3GLSQ',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				}),
				factory.create_embedded({
					'signer_public_key': '0675600F69145174A73300C71B35DF199C94B4EAD66E51F83A54F6C78FC2B7A7',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SCLC4CD32HATWYMAK45PN3YEUECZR4UYV7XFX6I',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [],
		}),
		'AggregateCompleteTransaction_14': factory.create({
			'signature': sc.Signature(
				'3E8ADFC708C78D43A23E34647FAC45396B0AAB4FE76392BCB5E9DA48DEB59C01'
				'799629A4E49A9E4AAF3201525DA44902217DAF85F530701F3711164CC711010F'),
			'signer_public_key': '44D2E54915FD2ECAF79B8895C4B454B4245B2509207EC8C490A66BD699D2D1E0',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '8A50B8F1A89BFC88B5459514935CF03A42590EF0B5ABA08C3516EEC54B984A89',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '6CADBD0098519C6BA27C49927DC1B626960F135FF5824293FFAF42066F8C8233',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SBYFSAQQF2EX3CEHI6FGD4F22XXGA4DB4MK3WYI',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [],
		}),
		'AggregateCompleteTransaction_15': factory.create({
			'signature': sc.Signature(
				'32D56CCF5DD17E5CB0BBD36709BB81C40C039A4357D073FB12D2E4B7723E78AD'
				'2BF8F11F061CDCB7084F19447BE0C0787B6E6440857E62519D28877F0F1A1602'),
			'signer_public_key': 'A1DE029466D3728ABA901B2F0BB1E73A70524537189F576AB9ED52FAB41959AF',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': 'D6B4830385A4701925CBF4414829BC8E92696A70A06BF70BCF70CA8C3F99162C',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'CB0778C7D596F0AD608F479E686EDB284A7330291A31E25070508898AE57EDCC',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SD2VY6KN7KQZFZTKZCSXMBVNDXU42EH6MCU5QAA',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				}),
				factory.create_embedded({
					'signer_public_key': 'A1DE029466D3728ABA901B2F0BB1E73A70524537189F576AB9ED52FAB41959AF',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SCOUVJGGRTMFP2X2RIUPG4O4VU32QVAVUNSC62I',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [],
		}),
		'AggregateCompleteTransaction_16': factory.create({
			'signature': sc.Signature(
				'32D56CCF5DD17E5CB0BBD36709BB81C40C039A4357D073FB12D2E4B7723E78AD'
				'2BF8F11F061CDCB7084F19447BE0C0787B6E6440857E62519D28877F0F1A1602'),
			'signer_public_key': 'A1DE029466D3728ABA901B2F0BB1E73A70524537189F576AB9ED52FAB41959AF',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': 'D6B4830385A4701925CBF4414829BC8E92696A70A06BF70BCF70CA8C3F99162C',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'CB0778C7D596F0AD608F479E686EDB284A7330291A31E25070508898AE57EDCC',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SD2VY6KN7KQZFZTKZCSXMBVNDXU42EH6MCU5QAA',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				}),
				factory.create_embedded({
					'signer_public_key': 'A1DE029466D3728ABA901B2F0BB1E73A70524537189F576AB9ED52FAB41959AF',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SCOUVJGGRTMFP2X2RIUPG4O4VU32QVAVUNSC62I',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765'),
				})
			],
			'cosignatures': [
				_create_cosignature(
					'CB0778C7D596F0AD608F479E686EDB284A7330291A31E25070508898AE57EDCC',
					'16AE9A3F67BB5B3D6C613E5B67E3DDBD8B771F64A973ACF71DC072CC3F6EEE7C7900FCF02472353E39A80F4F4180C38580227C92E882EDD89A0629F88A37BA0A')
			]
		}),
		'AggregateCompleteTransaction_17': factory.create({
			'signature': sc.Signature(
				'6B71F5C71EF109939916DBA83924679B93F3CF72F084AD499A0FCD3E75B1CA81'
				'F97E43E18B23830771909FF920BB69FEB11160E6230C080C57C196C095B8C801'),
			'signer_public_key': '37962F00E294752191B622118BDE0BBA4E1837711C62303A02AC394CC50384B4',
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '208D00C01487C8DE8ED820DF2C8BEEA36522E5AD8FC8554021D0CCE0BBCD61A1',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': 'B694186EE4AB0558CA4AFCFDD43B42114AE71094F5A1FC4A913FE9971CACD21D',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SCIVWAPBISHFABANFHBVJVCAYVVXTH4L5B7TIQY',
					'mosaics': [],
					'message': unhexlify('00746573742D6D657373616765')
				})
			],
			'cosignatures': [
				_create_cosignature(
					'48053805A88A714194E24E520D2416DDBBE1CD120C36BE7EB0BD4F9B39BB8A0C',
					'9B80C94CF5EA4928A11ACAE561292EB03FA43942EB48F0C1D30D7F67262D001C07A6E3741C1A08EF8CDACC849D1893B75D93D8DA2C13DC4A9B08BC032979E606'),
				_create_cosignature(
					'A980D122695475FC24456100579C3030BE0AFC1551E0ED53B66BDA1847AF9C5A',
					'9005335A1D770BFB14F8CF7CD04296C868E7B93D01724775D36AE22983E586D9BCAB0EC59E0AC61F1539F47E2C993D32A55CAFA847E4294C84061B28D5F7510C')
			]
		}),
		'AggregateCompleteTransaction_18': factory.create({
			'version': 0x1,
			'network': 'testnet',
			'type': 'aggregate_complete_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': 'F773DA9734E852B72FD07B6F5A0FCC3E5B4E502F91959C413E148E3349BBA6C5',
			'transactions': [
				factory.create_embedded({
					'signer_public_key': '67C2EC49DF614757B22153909EEF2FFF09D5EA5CF3DB4B7B414C11083A76829B',
					'version': 0x1,
					'network': 'testnet',
					'type': 'transfer_transaction',
					'recipient_address': 'SDRZKZUE2S32TIPY4PZ5PDMDIQIWFUBQBVLDUSA',
					'mosaics': [
						{'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000989680}
					],
					'message': unhexlify('00')
				})
			],
			'cosignatures': []
		})
	}
