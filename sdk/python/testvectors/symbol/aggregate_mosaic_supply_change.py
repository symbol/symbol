from binascii import unhexlify

from symbolchain import sc


def generate(factory):
	embedded_transactions_6 = [
		factory.create_embedded({
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 0x1,
			'type': 'transfer_transaction',
			'recipient_address': 'SDUP5PLHDXKBX3UU5Q52LAY4WYEKGEWC6IB3VBA',
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D657373616765')
		}),
		factory.create_embedded({
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 0x1,
			'type': 'mosaic_supply_change_transaction',
			'mosaic_id': 0x57701A9B6E746988,
			'action': 'increase',
			'delta': 0xA
		})
	]

	# TODO: seems senseless
	embedded_transactions_7 = [
		factory.create_embedded({
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 0x1,
			'type': 'transfer_transaction',
			'recipient_address': 'SDZWZJUAYNOWGBTCUDBY3SE5JF4NCC2RDM6SIGQ',
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D657373616765')
		}),
		factory.create_embedded({
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 0x1,
			'type': 'mosaic_supply_change_transaction',
			'mosaic_id': 0x57701A9B6E746988,
			'action': 'increase',
			'delta': 0xA
		})
	]

	cosignature_7_1 = sc.Cosignature()
	cosignature_7_1.signer_public_key = sc.PublicKey('9A49366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456111')
	cosignature_7_1.signature = sc.Signature(
		'AAA9366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456111'
		'AAA9366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456111')

	cosignature_7_2 = sc.Cosignature()
	cosignature_7_2.signer_public_key = sc.PublicKey('9A49366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456222')
	cosignature_7_2.signature = sc.Signature(
		'BBB9366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456222'
		'BBB9366406ACA952B88BADF5F1E9BE6CE4968141035A60BE503273EA65456222')

	embedded_transactions_8_and_9 = [
		factory.create_embedded({
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 0x1,
			'type': 'transfer_transaction',
			'recipient_address': 'SDZWZJUAYNOWGBTCUDBY3SE5JF4NCC2RDM6SIGQ',
			'mosaics': [
				{'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
			],
			'message': unhexlify('00536F6D65204D657373616765')
		}),
		factory.create_embedded({
			'signer_public_key': 'F6503F78FBF99544B906872DDB392F4BE707180D285E7919DBACEF2E9573B1E6',
			'version': 0x1,
			'type': 'mosaic_supply_change_transaction',
			'mosaic_id': 0x57701A9B6E746988,
			'action': 'increase',
			'delta': 0xA
		})
	]

	return {
		'AggregateBondedTransaction_6': factory.create({
			'type': 'aggregate_bonded_transaction',
			'deadline': 1,
			'transactions_hash': 'B4C97320255A2F755F6BE2F4DDAC0BB3EBDD25508DBE460EA6988366F404706A',
			'transactions': embedded_transactions_6
		}),
		'AggregateBondedTransaction_7': factory.create({
			'type': 'aggregate_bonded_transaction',
			'deadline': 1,
			'transactions_hash': '6C610D61B3E6839AE85AC18465CF6AD06D8F17A4F145F720BD324880B4FBB12B',
			'transactions': embedded_transactions_7,
			'cosignatures': [cosignature_7_1, cosignature_7_2]
		}),
		'AggregateBondedTransaction_8': factory.create({
			'type': 'aggregate_bonded_transaction',
			'deadline': 1,
			'transactions_hash': '6C610D61B3E6839AE85AC18465CF6AD06D8F17A4F145F720BD324880B4FBB12B',
			'transactions': embedded_transactions_8_and_9
		})
	}
