import { PrivateKey } from 'symbol-sdk';
import { SymbolFacade, NetworkTimestamp } from 'symbol-sdk/symbol';

// Initialize facade
const facade = new SymbolFacade('testnet');

// Set up the signing key pair
const privateKey = process.env.PRIVATE_KEY || '0000000000000000000000000000000000000000000000000000000000000000';
const keyPair = new SymbolFacade.KeyPair(new PrivateKey(privateKey));

// Set up transaction parameters
const currentTime = new NetworkTimestamp(1000000n); // Current network time
const deadline = currentTime.addHours(2).timestamp;
const recipientAddress = 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I';

// Create a transaction
const transaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: keyPair.publicKey.toString(),
	deadline: deadline,
	recipientAddress: recipientAddress,
	mosaics: [{ mosaicId: 0x6BED913FA20223F8n, amount: 1_000_000n }]
});

// Sign the transaction
const signature = facade.signTransaction(keyPair, transaction);
facade.transactionFactory.static.attachSignature(transaction, signature);

// Get the transaction hash
const transactionHash = facade.hashTransaction(transaction).toString().toUpperCase();

console.log(`Transaction hash: ${transactionHash}`);

