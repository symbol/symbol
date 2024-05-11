import { PrivateKey, PublicKey } from '../../src/index.js';
import {
	Address, SymbolFacade, descriptors, models
} from '../../src/symbol/index.js';

const signAndPrint = (facade, transaction) => {
	console.log('created Symbol transaction:');
	console.log(transaction.toString());

	const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
	const signature = facade.signTransaction(new facade.static.KeyPair(privateKey), transaction);

	const jsonPayload = facade.transactionFactory.static.attachSignature(transaction, signature);

	console.log('prepared Symbol JSON payload:');
	console.log(jsonPayload);
	console.log('');
};

(() => {
	const rawDescriptor = {
		type: 'transfer_transaction_v1',
		signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
		fee: 1000000n,
		deadline: 41998024783n,
		recipientAddress: 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
		mosaics: [
			{ mosaicId: 0x7CDF3B117A3C40CCn, amount: 1000000n }
		],
		message: 'hello symbol'
	};

	console.log('*** EXAMPLE CONSTRUCTION FROM UNTYPED MAP ***');
	const facade = new SymbolFacade('testnet');
	const transaction = facade.transactionFactory.create(rawDescriptor);
	signAndPrint(facade, transaction);

	console.log('*** EXAMPLE CONSTRUCTION FROM UNTYPED MAP - EMBEDDED ***');
	const {
		fee,
		deadline,
		...rawEmbeddedDescriptor
	} = rawDescriptor; // remove fee and deadline from rawDescriptor
	const embeddedTransaction = facade.transactionFactory.createEmbedded(rawEmbeddedDescriptor);
	signAndPrint(facade, embeddedTransaction);
})();

(() => {
	const typedDescriptor = new descriptors.TransferTransactionV1Descriptor(
		new Address('TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I'),
		[
			new descriptors.UnresolvedMosaicDescriptor(new models.UnresolvedMosaicId(0x7CDF3B117A3C40CCn), new models.Amount(1000000n))
		],
		'hello symbol'
	);

	console.log('*** EXAMPLE CONSTRUCTION FROM TYPED DESCRIPTOR OBJECT AND FACADE ***');
	const facade = new SymbolFacade('testnet');
	const transaction = facade.createTransactionFromTypedDescriptor(
		typedDescriptor,
		new PublicKey('87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8'),
		100,
		60 * 60
	);
	signAndPrint(facade, transaction);

	console.log('*** EXAMPLE CONSTRUCTION FROM TYPED DESCRIPTOR OBJECT AND FACADE - EMBEDDED ***');
	const embeddedTransaction = facade.createEmbeddedTransactionFromTypedDescriptor(
		typedDescriptor,
		new PublicKey('87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8')
	);

	signAndPrint(facade, embeddedTransaction);
})();
