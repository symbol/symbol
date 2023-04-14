from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.NemFacade import NemFacade

facade = NemFacade('testnet')
transaction = facade.transaction_factory.create({
	'type': 'transfer_transaction_v1',
	'signer_public_key': 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
	'fee': 0x186A0,
	'timestamp': 191205516,
	'deadline': 191291916,
	'recipient_address': 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
	'amount': 5100000
})

print('created NEM transaction:')
print(transaction)

private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
signature = facade.sign_transaction(facade.KeyPair(private_key), transaction)

json_payload = facade.transaction_factory.attach_signature(transaction, signature)

print('prepared NEM JSON payload:')
print(json_payload)
