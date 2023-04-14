# Symbol-SDK

[![lint][sdk-javascript-lint]][sdk-javascript-job] [![test][sdk-javascript-test]][sdk-javascript-job] [![vectors][sdk-javascript-vectors]][sdk-javascript-job] [![][sdk-javascript-cov]][sdk-javascript-cov-link] [![][sdk-javascript-package]][sdk-javascript-package-link]

[sdk-javascript-job]: https://jenkins.symboldev.com/blue/organizations/jenkins/Symbol%2Fgenerated%2Fsymbol%2Fjavascript/activity?branch=dev
[sdk-javascript-lint]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fjavascript%2Fdev%2F&config=sdk-javascript-lint
[sdk-javascript-test]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fjavascript%2Fdev%2F&config=sdk-javascript-test
[sdk-javascript-vectors]: https://jenkins.symboldev.com/buildStatus/icon?job=Symbol%2Fgenerated%2Fsymbol%2Fjavascript%2Fdev%2F&config=sdk-javascript-vectors
[sdk-javascript-cov]: https://codecov.io/gh/symbol/symbol/branch/dev/graph/badge.svg?token=SSYYBMK0M7&flag=sdk-javascript
[sdk-javascript-cov-link]: https://codecov.io/gh/symbol/symbol/tree/dev/sdk/javascript
[sdk-javascript-package]: https://img.shields.io/npm/v/symbol-sdk
[sdk-javascript-package-link]: https://www.npmjs.com/package/symbol-sdk

JavaScript SDK for interacting with the Symbol and NEM blockchains.

Most common functionality is grouped under facades so that the same programming paradigm can be used for interacting with both Symbol and NEM.

## Sending a Transaction

To send a transaction, first create a facade for the desired network:

_Symbol_
```javascript
import symbolSdk from 'symbol-sdk';

const facade = new symbolSdk.facade.SymbolFacade('testnet');
```

_NEM_
```javascript
import symbolSdk from 'symbol-sdk';

const facade = new symbolSdk.facade.NemFacade('testnet');
````

Second, describe the transaction using JavaScript object syntax. For example, a transfer transaction can be described as follows:

_Symbol_
```javascript
const transaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
	fee: 1000000n,
	deadline: 41998024783n,
	recipientAddress: 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
	mosaics: [
		{ mosaicId: 0x7CDF3B117A3C40CCn, amount: 1000000n }
	]
});
```

_NEM_
```javascript
const transaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
	fee: 0x186A0n,
	timestamp: 191205516,
	deadline: 191291916,
	recipientAddress: 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
	amount: 5100000n
});
````

Third, sign the transaction and attach the signature:


```javascript
const privateKey = new symbolSdk.PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
const signature = facade.signTransaction(new facade.constructor.KeyPair(privateKey), transaction);

const jsonPayload = facade.transactionFactory.constructor.attachSignature(transaction, signature);;
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
