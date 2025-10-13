import { PrivateKey, PublicKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateMosaicAliasId,
	MessageEncoder
} from 'symbol-sdk/symbol';

// Configuration
const NODE_URL = process.env.NODE_URL ||
	'https://001-sai-dual.symboltest.net:3001';

// Set up sender and recipient accounts
const facade = new SymbolFacade('testnet');
const senderPrivateKey = process.env.PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const senderKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(senderPrivateKey));
const senderAddress = facade.network.publicKeyToAddress(
	senderKeyPair.publicKey);

// Set up recipient (using public key only)
const recipientPublicKeyString = process.env.RECIPIENT_PUBLIC_KEY ||
	'D04AB232742BB4AB3A1368BD4615E4E6D0224AB71A016BAF8520A332C9778737';

const recipientPublicKey = new PublicKey(recipientPublicKeyString);
const recipientAddress = facade.network.publicKeyToAddress(recipientPublicKey);
console.log('Sender address:', senderAddress.toString());
console.log('Recipient address:', recipientAddress.toString(), '\n');

try {
	// ===== PLAIN TEXT MESSAGE =====
	console.log('=== Sending Plain Text Message ===');

	// Fetch current network time
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	let timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log('  Network time:', timestamp.timestamp, 'ms since nemesis');

	// Fetch recommended fees
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);

	// Create a plain text message
	const plainMessage = new TextEncoder().encode('Hello, Symbol!');
	console.log('Plain message:', new TextDecoder().decode(plainMessage));

	// Build transfer transaction with plain message
	let transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: senderKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: recipientAddress.toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n	// 1 XYM
		}],
		message: plainMessage
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);

	// Sign and announce the transaction
	let signature = facade.signTransaction(senderKeyPair, transaction);
	let jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	let transactionHash = facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	let announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('Plain message transaction announced\n');

	// Wait a moment before sending the next transaction
	await new Promise(resolve => setTimeout(resolve, 2000));

	// ===== ENCRYPTED MESSAGE =====
	console.log('=== Sending Encrypted Message ===');

	// Fetch updated network time
	const timeResponse2 = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON2 = await timeResponse2.json();
	timestamp = new NetworkTimestamp(
		timeJSON2.communicationTimestamps.receiveTimestamp);

	// Create a message encoder with sender's key pair
	const messageEncoder = new MessageEncoder(senderKeyPair);

	// Encrypt the message using recipient's public key
	const secretMessage = new TextEncoder().encode(
		'This is a secret message!');
	const encryptedPayload = messageEncoder.encode(
		recipientPublicKey, secretMessage
	);
	console.log('Original message:', new TextDecoder().decode(secretMessage));
	const hex = Buffer.from(encryptedPayload).toString('hex');
	console.log('Encrypted payload:', hex);

	// Build transfer transaction with encrypted message
	transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: senderKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: recipientAddress.toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n	// 1 XYM
		}],
		message: encryptedPayload
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);

	// Sign and announce the transaction
	signature = facade.signTransaction(senderKeyPair, transaction);
	jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	transactionHash = facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('Encrypted message transaction announced\n');

	// ===== DECRYPTING MESSAGE =====
	console.log('=== Decrypting Message ===');

	// Sender can decrypt using recipient's public key
	const result = messageEncoder.tryDecode(
		recipientPublicKey, encryptedPayload);

	if (result.isDecoded) {
		console.log('Sender verified encrypted message:',
			new TextDecoder().decode(result.message));
	} else {
		console.log('Sender failed to decrypt message');
	}

	// The recipient would decrypt using their private key:
	// const recipientEncoder = new MessageEncoder(recipientKeyPair);
	// const result = recipientEncoder.tryDecode(
	//	senderKeyPair.publicKey, encryptedPayload
	// );

	console.log('\nBoth transactions have been sent successfully!');

} catch (error) {
	console.error(
		`Error: ${error.message} | Cause: ${error.cause?.code ?? 'unknown'}`
	);
}
