# Symbol-SDK

[![lint][sdk-python-lint]][sdk-python-job] [![test][sdk-python-test]][sdk-python-job] [![vectors][sdk-python-vectors]][sdk-python-job] [![][sdk-python-cov]][sdk-python-cov-link] [![][sdk-python-package]][sdk-python-package-link]

[sdk-python-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fpython/activity?branch=dev
[sdk-python-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fpython%2Fdev%2F&config=sdk-python-lint
[sdk-python-build]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fpython%2Fdev%2F&config=sdk-python-build
[sdk-python-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fpython%2Fdev%2F&config=sdk-python-test
[sdk-python-examples]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fpython%2Fdev%2F&config=sdk-python-examples
[sdk-python-vectors]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fpython%2Fdev%2F&config=sdk-python-vectors
[sdk-python-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=sdk-python
[sdk-python-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/sdk/python
[sdk-python-package]: https://img.shields.io/pypi/v/symbol-sdk-python
[sdk-python-package-link]: https://pypi.org/project/symbol-sdk-python

Python SDK for interacting with the Symbol and NEM blockchains.

Most common functionality is grouped under facades so that the same programming paradigm can be used for interacting with both Symbol and NEM.

## Sending a Transaction

To send a transaction, first create a facade for the desired network:

_Symbol_
```python
from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade


facade = SymbolFacade('testnet')
```

_NEM_
```python
from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade

facade = SymbolFacade('testnet')
````

Second, describe the transaction using a Python dictionary. For example, a transfer transaction can be described as follows:

_Symbol_
```python
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
```

_NEM_
```python
transaction = facade.transaction_factory.create({
	'type': 'transfer_transaction_v1',
	'signer_public_key': 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
	'fee': 0x186A0,
	'timestamp': 191205516,
	'deadline': 191291916,
	'recipient_address': 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
	'amount': 5100000
})
````

Third, sign the transaction and attach the signature:


```python
private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
signature = facade.sign_transaction(facade.KeyPair(private_key), transaction)

json_payload = facade.transactionFactory.attachSignature(transaction, signature)
```

Finally, send the payload to the desired network using the specified node endpoint:

_Symbol_: PUT `/transactions`
<br>
_NEM_: POST `/transaction/announce`


## NEM Cheat Sheet

In order to simplify the learning curve for NEM and Symbol usage, the SDK uses Symbol terminology for shared Symbol and NEM concepts.
Where appropriate, NEM terminology is replaced with Symbol terminology, including the names of many of the NEM transactions.
The mapping of NEM transactions to SDK descriptors can be found in the following table:

| NEM name (used in docs) | SDK descriptor name|
|--- |--- |
| ImportanceTransfer transaction | `account_key_link_transaction_v1` |
| MosaicDefinitionCreation transaction | `mosaic_definition_transaction_v1` |
| MosaicSupplyChange transaction | `mosaic_supply_change_transaction_v1` |
| MultisigAggregateModification transaction | `multisig_account_modification_transaction_v1`<br>`multisig_account_modification_transaction_v2` |
| MultisigSignature transaction or Cosignature transaction | `cosignature_v1` |
| Multisig transaction | `multisig_transaction_v1` |
| ProvisionNamespace transaction | `namespace_registration_transaction_v1` |
| Transfer transaction | `transfer_transaction_v1`<br>`transfer_transaction_v2` |
