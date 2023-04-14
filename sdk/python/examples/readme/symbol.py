from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade

facade = SymbolFacade('testnet')
transaction = facade.transaction_factory.create({
	'type': 'transfer_transaction_v1',
	'signer_public_key': '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
	'fee': 1000000,
	'deadline': 41998024783,
	'recipient_address': 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
	'mosaics': [
		{'mosaic_id': 0x7CDF3B117A3C40CC, 'amount': 1000000}
	]
})

print('created Symbol transaction:')
print(transaction)

private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
signature = facade.sign_transaction(facade.KeyPair(private_key), transaction)

json_payload = facade.transaction_factory.attach_signature(transaction, signature)

print('prepared Symbol JSON payload:')
print(json_payload)
