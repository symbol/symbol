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


This is minimal Javascript SDK for Symbol and NEM.
The architecture and programming paradigm of this SDK are consistent with those for other languages.
Old typescript SDK - [symbol-sdk v2.0.0](https://www.npmjs.com/package/symbol-sdk/v/2.0.0) - has been deprecated.

Note, at this point NEM support should be considered **experimental**.

## SDK

NEM and Symbol features are grouped under facades.

1. Transactions can be created using so called 'descriptors'.

Symbol:

```javascript
const { SymbolFacade } = require('symbol-sdk').facade;
...
const facade = new SymbolFacade('testnet');
const transaction = facade.transactionFactory.create({
	type: 'transfer_transaction',
	signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
	fee: 1000000n,
	deadline: 41998024783n,
	recipientAddress: 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
	mosaics: [
		{ mosaicId: 0x7CDF3B117A3C40CCn, amount: 1000000n }
	]
});
```

NEM:

```javascript
const { NemFacade } = require('symbol-sdk').facade;
...
const facade = new NemFacade('testnet');

const transaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
	fee: 0x186A0n,
	timestamp: 191205516,
	deadline: 191291916,
	recipientAddress: 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
	amount: 5100000n
});
```

2. Signature should always be attached via transaction factory, sample:
```javascript
const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
const signature = facade.signTransaction(new NemFacade.KeyPair(privateKey), transaction);

const jsonPayload = facade.transactionFactory.constructor.attachSignature(transaction, signature);
```

`jsonPayload` can then be pushed to appropriate REST endpoints:
 * nem: POST `/transaction/announce`
 * symbol: PUT `/transactions`

3. NEM transaction names are aligned with the names used in catapult schemas [catapult schemas](catbuffer/schemas).

Cheat sheet:

| "old" name (used in docs) | descriptor name|
|--- |--- |
| ImportanceTransfer transaction | `account_key_link_transaction` |
| MosaicDefinitionCreation transaction | `mosaic_definition_transaction` |
| MosaicSupplyChange transaction | `mosaic_supply_change_transaction` |
| MultisigAggregateModification transaction | `multisig_account_modification_transaction_v1` |
| MultisigAggregateModification transaction | `multisig_account_modification_transaction` |
| MultisigSignature transaction or Cosignature transaction | `cosignature` |
| Multisig transaction | `multisig_transaction` |
| ProvisionNamespace transaction | `namespace_registration_transaction` |
| Transfer transaction | `transfer_transaction_v1` |
| Transfer transaction | `transfer_transaction` |
