from binascii import unhexlify

from symbolchain import sc

SAMPLE_ADDRESS_ALIAS = 'SFIXOYLI2JBFPWAAAAAAAAAAAAAAAAAAAAAAAAA'
SAMPLE_ADDRESS = 'SDZWZJUAYNOWGBTCUDBY3SE5JF4NCC2RDM6SIGQ'


def _generate_normal(factory):
	descriptors = {
		'TransferTransaction_1': {
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x0000000000000064, 'amount': 0x0000000000000002},
				{'mosaic_id': 0x00000000000000C8, 'amount': 0x0000000000000001}
			],
			'message': unhexlify('00')
		},
		'TransferTransaction_2': {
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS_ALIAS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D65737361676520E6BCA2E5AD97')
		},
		'TransferTransaction_4': {
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00')
		},
		'TransferTransaction_6': {
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x67F2B76F28BD36BA, 'amount': 0x0000000000000003},
				{'mosaic_id': 0x77A1969932D987D7, 'amount': 0x0000000000000002},
				{'mosaic_id': 0xD525AD41D95FCF29, 'amount': 0x0000000000000001}
			],
			'message': unhexlify('00')
		},
		'TransferTransaction_7': {
			'type': 'transfer_transaction',
			'deadline': 0x000001759B7BDC06,
			'recipient_address': 'SDSXIDBWPZ3DTC3B2TISCZ3X5DPB3Q54EXIS4ZQ',
			'mosaics': [],
			'message': unhexlify(
				'FE453230313733353736313830324146453732373439363532413837413643453336343945383645314544353344333145393541333345363044394530364435'
				'43384145354546373731373233354530393632463031334445323435373043313239463145333834323444304135423344343842373642334646343934334142'
				'41434544393437383231373637314644464430323730443046384344353437384430313734313530453246433930393638363242383945374332384444373246'
				'46314435333441393831384335314234383330324632414646303143303045423844344544464133463741314235384245343743323444364445433830453739'
				'41324542433037313432434530384343304546334535354242313345383244363130424236423535463933354139363831323335434539413045434336364231'
				'43314138334643414639334434353546313943383537364631443342384336433134344637414135414136434234444333333242443035314441394233434541'
				'304330363437464646')
		},
		'TransferTransaction_8': {
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': 'SGEN27LSEJ7MVZYAAAAAAAAAAAAAAAAAAAAAAAA',
			'mosaics': [
				{'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000000000001}
			],
			'message': unhexlify('00746573742D6D657373616765')
		},
		'TransferTransaction_9': {
			'type': 'transfer_transaction',
			'signer_public_key': '2134E47AEE6F2392A5B3D1238CD7714EABEB739361B7CCF24BAE127F10DF17F2',
			'version': 1,
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D65737361676520E6BCA2E5AD97')
		},
		'TransferTransaction_11': {
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			]
		},
		'TransferTransaction_13': {
			'version': 0x1,
			'network': 'testnet',
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': 'SDRUEI436GK6AE4E5NXMGU3DTVFGQ6RJIZVHM2A',
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D657373616765')
		},
		'TransferTransaction_14': {
			'signature': sc.Signature(
				'182AF94BD22DF48D81EE9CD758CA14D79FEA9362DBF5339325A8244F3EB8DF20'
				'E6432DA8FA9E0B2B5F2AB8A815CE29453795B6298B90F46959EC1FAA377A880C'),
			'signer_public_key': '2134E47AEE6F2392A5B3D1238CD7714EABEB739361B7CCF24BAE127F10DF17F2',
			'version': 0x1,
			'network': 'testnet',
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': 'SBXE7SPEUVVU44HX7IYNA3MS2H5PMK2XNEC4SUY',
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D657373616765')
		},
		'TransferTransaction_15': {
			'version': 0x1,
			'network': 'testnet',
			'type': 'transfer_transaction',
			'deadline': 0x0000000000000001,
			'recipient_address': 'SBXE7SPEUVVU44HX7IYNA3MS2H5PMK2XNEC4SUY',
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D657373616765')
		},
		'TransferTransaction_16': {
			'version': 0x1,
			'network': 'testnet',
			'type': 'transfer_transaction',
			'deadline': 0x000000000000022B,
			'recipient_address': 'QATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA367I6OQ',
			'mosaics': [
				{'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x0000000005F5E100}
			],
			'message': unhexlify('00746573742D6D657373616765')
		},
	}

	return {k: factory.create(v) for (k, v) in descriptors.items()}


def _generate_aggregates(factory):
	embedded_transactions_23 = [
		factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': 'C613FD58985D6BBEDFD9BDFD1CA9D8C72B5B30E4A3BC99114D0BFCAB133E395A',
			'version': 1,
			'recipient_address': SAMPLE_ADDRESS_ALIAS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D65737361676520E6BCA2E5AD97')
		})
	]
	embedded_transactions_24 = [
		factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': 'AF07FF69FF95F4FCC5B83CC750EC20E37F2E9968644FA18D603A3F1CDC6A7E46',
			'version': 1,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00')
		})
	]

	embedded_transactions_25 = [
		factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': 'C898C0939F037B6F2D0ECCA8BC80783B6901ED744ADDC1B0C985C9CC27A11B7E',
			'version': 1,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x00000B39C6051367, 'amount': 0x000000000000012C},
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D65737361676520E6BCA2E5AD97')
		})
	]

	embedded_transactions_26 = [
		factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': '216152AB1229A8CC65811F325B9852EA28C1BC223ABCF2FD10A9845B797DA9DB',
			'version': 1,
			'recipient_address': SAMPLE_ADDRESS,
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			]
		})
	]

	embedded_transactions_29 = [
		factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': '9801508C58666C746F471538E43002B85B1CD542F9874B2861183919BA8787B6',
			'version': 1,
			'recipient_address': 'QATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA367I6OQ',
			'mosaics': [],
			'message': unhexlify('00746573742D6D657373616765')
		})
	]

	return {
		'AggregateBondedTransaction_23': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'E1BC0003684C91F467AD20A6312AFC8FC1E578B633DDC53AF0C7CED047C43393',
			'transactions': embedded_transactions_23
		}),
		'AggregateBondedTransaction_24': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'CF4DE1675E204A2D06C2B468F37EE71DC4DF918CB0A6D6783FBD142AD07F774C',
			'transactions': embedded_transactions_24
		}),
		'AggregateBondedTransaction_25': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': '45D0CD037E228D93D8D4A3CFFCBE09A7308F9E9B3B357B1D7DD55E80B48111CF',
			'transactions': embedded_transactions_25
		}),
		'AggregateBondedTransaction_26': factory.create({
			'type': 'aggregate_bonded_transaction',
			'fee': 10,
			'deadline': 1,
			'transactions_hash': 'A733A9FB12496D8E8BA9907E7395BE54E2EC8AA9DE0915B3F8C14E38F96A678E',
			'transactions': embedded_transactions_26
		}),
		'AggregateBondedTransaction_29': factory.create({
			'type': 'aggregate_bonded_transaction',
			'deadline': 0x000000000000022B,
			'transactions_hash': '887DE1026EA57A350FF35BD13163D4C8D5E149A3DC281D3686400AD2906D15BC',
			'transactions': embedded_transactions_29
		}),
		'AggregateBondedTransaction_30': factory.create({
			'type': 'aggregate_bonded_transaction',
			'deadline': 0x000000000000022B,
			'transactions_hash': '0000000000000000000000000000000000000000000000000000000000000000',
			'transactions': []
		}),
		'AggregateBondedTransaction_31': factory.create({
			'type': 'aggregate_bonded_transaction',
			'deadline': 0x0000000000000001,
			'transactions_hash': '0000000000000000000000000000000000000000000000000000000000000000',
			'transactions': []
		}),
	}


def generate(factory):
	transactions = _generate_normal(factory)
	agregates = _generate_aggregates(factory)
	return {**transactions, **agregates}
