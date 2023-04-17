import symbolSdk from '../../src/index.js';

const facade = new symbolSdk.facade.NemFacade('testnet');
const transaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
	fee: 0x186A0n,
	timestamp: 191205516,
	deadline: 191291916,
	recipientAddress: 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
	amount: 5100000n
});

console.log('created NEM transaction:');
console.log(transaction.toString());

const privateKey = new symbolSdk.PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
const signature = facade.signTransaction(new facade.constructor.KeyPair(privateKey), transaction);

const jsonPayload = facade.transactionFactory.constructor.attachSignature(transaction, signature);

console.log('prepared NEM JSON payload:');
console.log(jsonPayload);
