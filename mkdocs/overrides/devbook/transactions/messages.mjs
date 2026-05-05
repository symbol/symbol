import { PrivateKey, PublicKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	MessageEncoder
} from 'symbol-sdk/symbol';

// Configuration
const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Helper function to poll for confirmed transaction
async function retrieveConfirmedTransaction(hash, label) {
	console.log(`Polling for ${label} confirmation...`);
	let confirmed = false;
	let attempts = 0;
	const maxAttempts = 60;

	while (!confirmed && attempts < maxAttempts) {
		try {
			const response = await fetch(
				`${NODE_URL}/transactions/confirmed/${hash}`);
			if (response.ok) {
				confirmed = true;
				console.log(`  ${label} confirmed!`);
				return await response.json();
			}
		} catch (error) {
			// Transaction not yet confirmed
		}
		attempts++;
		await new Promise(resolve => setTimeout(resolve, 2000));
	}

	if (!confirmed) {
		throw new Error(
			`${label} not confirmed after ${maxAttempts} attempts`);
	}
}

// Set up sender and recipient accounts [>step-1]
const facade = new SymbolFacade('testnet');

const senderPrivateKeyString = process.env.SENDER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const senderKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(senderPrivateKeyString));
const senderAddress = facade.network.publicKeyToAddress(
	senderKeyPair.publicKey);

const recipientPrivateKeyString = process.env.RECIPIENT_PRIVATE_KEY ||
	'1111111111111111111111111111111111111111111111111111111111111111';
const recipientKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(recipientPrivateKeyString));
const recipientAddress = facade.network.publicKeyToAddress(
	recipientKeyPair.publicKey);

console.log('Sender address:', senderAddress.toString());
console.log('Recipient address:', recipientAddress.toString(), '\n');
// [<step-1]
// Fetch current network time
const timePath = '/node/time';
console.log('Fetching current network time from', timePath);
const timeResponse = await fetch(`${NODE_URL}${timePath}`);
const timeJSON = await timeResponse.json();
const timestamp = new NetworkTimestamp(
	timeJSON.communicationTimestamps.receiveTimestamp);
console.log('  Network time:', timestamp.timestamp,
	'ms since nemesis');

// Fetch recommended fees
const feePath = '/network/fees/transaction';
console.log('Fetching recommended fees from', feePath);
const feeResponse = await fetch(`${NODE_URL}${feePath}`);
const feeJSON = await feeResponse.json();
const medianMult = feeJSON.medianFeeMultiplier;
const minimumMult = feeJSON.minFeeMultiplier;
const feeMult = Math.max(medianMult, minimumMult);
console.log('  Fee multiplier:', feeMult, '\n');

// ===== PLAIN TEXT MESSAGE =====
console.log('==> Sending Plain Text Message'); // [>step-2]

// Create a plain text message
const plainMessage = new TextEncoder().encode('Hello, Symbol!');
console.log('Plain message:',
	new TextDecoder().decode(plainMessage));

// Build transfer transaction with plain message
const plainTransaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: senderKeyPair.publicKey.toString(),
	deadline: timestamp.addHours(2).timestamp,
	recipientAddress: recipientAddress.toString(),
	mosaics: [],
	message: plainMessage
}); // [<step-2]
plainTransaction.fee = new models.Amount(feeMult * plainTransaction.size);

// Sign and announce the transaction
const plainSignature = facade.signTransaction(
	senderKeyPair, plainTransaction);
const plainJsonPayload = facade.transactionFactory.static
	.attachSignature(plainTransaction, plainSignature);
const plainTransactionHash = facade.hashTransaction(
	plainTransaction).toString();
console.log('Transaction hash:', plainTransactionHash);

await fetch(`${NODE_URL}/transactions`, {
	method: 'PUT',
	headers: { 'Content-Type': 'application/json' },
	body: plainJsonPayload
});
console.log('Plain message transaction announced\n');

// ===== RECEIVING PLAIN TEXT MESSAGE =====
console.log('<== Receiving Plain Text Message'); // [>step-3]

// Wait for confirmation
const plainTxData = await retrieveConfirmedTransaction(
	plainTransactionHash, 'Plain message transaction');

// Decode plain message from confirmed transaction
const receivedPlainMessage = Buffer.from(
	plainTxData.transaction.message, 'hex');
console.log('Received plain message:',
	new TextDecoder().decode(receivedPlainMessage), '\n');
// [<step-3]
// ===== ENCRYPTED MESSAGE =====
console.log('==> Sending Encrypted Message'); // [>step-4]

// Create a message encoder with sender's key pair
const senderMessageEncoder = new MessageEncoder(senderKeyPair);

// Encrypt the message using recipient's public key
const secretMessage = new TextEncoder().encode(
	'This is a secret message!');
const encryptedPayload = senderMessageEncoder.encode(
	recipientKeyPair.publicKey, secretMessage
);
console.log('Original message:',
	new TextDecoder().decode(secretMessage));
const hex = Buffer.from(encryptedPayload).toString('hex');
console.log('Encrypted payload:', hex);

// Build transfer transaction with encrypted message
const encryptedTransaction = facade.transactionFactory.create({
	type: 'transfer_transaction_v1',
	signerPublicKey: senderKeyPair.publicKey.toString(),
	deadline: timestamp.addHours(2).timestamp,
	recipientAddress: recipientAddress.toString(),
	mosaics: [],
	message: encryptedPayload
}); // [<step-4]
encryptedTransaction.fee = new models.Amount(
	feeMult * encryptedTransaction.size);

// Sign and announce the transaction
const encryptedSignature = facade.signTransaction(
	senderKeyPair, encryptedTransaction);
const encryptedJsonPayload = facade.transactionFactory.static
.attachSignature(encryptedTransaction, encryptedSignature);
const encryptedTransactionHash = facade.hashTransaction(
	encryptedTransaction).toString();
console.log('Transaction hash:', encryptedTransactionHash);

await fetch(`${NODE_URL}/transactions`, {
	method: 'PUT',
	headers: { 'Content-Type': 'application/json' },
	body: encryptedJsonPayload
});
console.log('Encrypted message transaction announced\n');

// ===== RECEIVING ENCRYPTED MESSAGE =====
console.log('<== Receiving Encrypted Message'); // [>step-5]

// Wait for confirmation
const encryptedTxData = await retrieveConfirmedTransaction(
	encryptedTransactionHash, 'Encrypted message transaction');

// Decode encrypted message using recipient's private key
const recipientMessageEncoder = new MessageEncoder(recipientKeyPair);
const receivedEncryptedMessage = Buffer.from(
	encryptedTxData.transaction.message, 'hex');

// Get sender's public key from the transaction
const senderPublicKeyFromTx = new PublicKey(
	encryptedTxData.transaction.signerPublicKey);

const result = recipientMessageEncoder.tryDecode(
	senderPublicKeyFromTx, receivedEncryptedMessage);

if (result.isDecoded) {
	console.log('Recipient decrypted message:',
		new TextDecoder().decode(result.message));
} else {
	console.log('Recipient failed to decrypt message');
} // [<step-5]
