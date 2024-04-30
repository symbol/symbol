import { PrivateKey, PublicKey } from '../../src/index.js';
import {
	Address, NemFacade, descriptors, models
} from '../../src/nem/index.js';

const signAndPrint = (facade, transaction) => {
	console.log('created NEM transaction:');
	console.log(transaction.toString());

	const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
	const signature = facade.signTransaction(new facade.static.KeyPair(privateKey), transaction);

	const jsonPayload = facade.transactionFactory.static.attachSignature(transaction, signature);

	console.log('prepared NEM JSON payload:');
	console.log(jsonPayload);
	console.log('');
};

(() => {
	const rawDescriptor = {
		type: 'transfer_transaction_v1',
		signerPublicKey: 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
		fee: 0x186A0n,
		timestamp: 191205516,
		deadline: 191291916,
		recipientAddress: 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
		amount: 5100000n,
		message: {
			messageType: 1,
			message: 'hello nem'
		}
	};

	console.log('*** EXAMPLE CONSTRUCTION FROM UNTYPED MAP ***');
	const facade = new NemFacade('testnet');
	const transaction = facade.transactionFactory.create(rawDescriptor);
	signAndPrint(facade, transaction);
})();

(() => {
	const typedDescriptor = new descriptors.TransferTransactionV1Descriptor(
		new Address('TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW'),
		new models.Amount(5100000n),
		new descriptors.MessageDescriptor(models.MessageType.PLAIN, 'hello nem')
	);

	console.log('*** EXAMPLE CONSTRUCTION FROM TYPED DESCRIPTOR OBJECT AND FACADE ***');
	const facade = new NemFacade('testnet');
	const transaction = facade.createTransactionFromTypedDescriptor(
		typedDescriptor,
		new PublicKey('A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74'),
		0x186A0n,
		60 * 60
	);
	signAndPrint(facade, transaction);
})();
