import { PrivateKey, PublicKey } from '../../src/index.js';
import {
	Address,
	SymbolFacade,
	TransferTransactionV1Descriptor,
	TransferTransactionV1Descriptor2,
	UnresolvedMosaicDescriptor,
	UnresolvedMosaicDescriptor2,
	models
} from '../../src/symbol/index.js';
import { deepCompare } from '../../src/utils/arrayHelpers.js';

const facade = new SymbolFacade('testnet');

const buildTransactionFromDescriptor = descriptor => {
	const transaction = facade.transactionFactory.create(descriptor);

	console.log('created Symbol transaction:');
	console.log(transaction.toString());

	const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
	const signature = facade.signTransaction(new facade.constructor.KeyPair(privateKey), transaction);

	const jsonPayload = facade.transactionFactory.constructor.attachSignature(transaction, signature);

	console.log('prepared Symbol JSON payload:');
	console.log(jsonPayload);

	return signature;
};

// 1. original method [JS Object]
const descriptor1 = {
	type: 'transfer_transaction_v1',
	signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
	fee: 1000000n,
	deadline: 41998024783n,
	recipientAddress: 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
	mosaics: [
		{ mosaicId: 0x7CDF3B117A3C40CCn, amount: 1000000n }
	]
};
const signature1 = buildTransactionFromDescriptor(descriptor1);

// 2. sdk types + model types [type safe]
const descriptor2 = new TransferTransactionV1Descriptor(
	new PublicKey('87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8'),
	new models.Amount(1000000n),
	new models.Timestamp(41998024783n),
	new Address('TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I'),
	undefined,
	[
		new UnresolvedMosaicDescriptor(new models.UnresolvedMosaicId(0x7CDF3B117A3C40CCn), new models.Amount(1000000n))
	]
);
const signature2 = buildTransactionFromDescriptor(descriptor2.toMap());

// 3. sdk types + JS primitives [type safe]
const descriptor3 = new TransferTransactionV1Descriptor2(
	new PublicKey('87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8'),
	1000000n,
	41998024783n,
	new Address('TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I'),
	undefined,
	[
		new UnresolvedMosaicDescriptor2(0x7CDF3B117A3C40CCn, 1000000n)
	]
);
const signature3 = buildTransactionFromDescriptor(descriptor3.toMap());

console.log('--- --- ---');
console.log(signature1);
console.log(signature2);
console.log(signature3);
console.log(0 === deepCompare(signature1.bytes, signature2.bytes));
console.log(0 === deepCompare(signature1.bytes, signature3.bytes));
