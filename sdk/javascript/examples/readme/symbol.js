import symbolSdk from '../../src/index.js';

const facade = new symbolSdk.facade.SymbolFacade('testnet');
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

console.log('created Symbol transaction:');
console.log(transaction.toString());

const privateKey = new symbolSdk.PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
const signature = facade.signTransaction(new facade.constructor.KeyPair(privateKey), transaction);

const jsonPayload = facade.transactionFactory.constructor.attachSignature(transaction, signature);

console.log('prepared Symbol JSON payload:');
console.log(jsonPayload);
