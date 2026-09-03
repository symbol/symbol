import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
	calculateTransactionFee,
	generateMosaicAliasId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Helper function to announce a transaction
async function announceTransaction(payload, label) {
	console.log(`Announcing ${label} to /transactions`);
	const response = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: payload
	});
	console.log('  Response:', await response.text());
}

// Helper function to wait for transaction confirmation
async function waitForConfirmation(transactionHash, label) {
	console.log(`Waiting for ${label} confirmation...`);
	for (let attempt = 0; 60 > attempt; attempt++) {
		await new Promise(resolve => { setTimeout(resolve, 1000); });
		const response = await fetch(
			`${NODE_URL}/transactionStatus/${transactionHash}`);
		if (!response.ok) {
			if (404 === response.status) {
				console.log('  Transaction status: unknown');
				continue;
			}
			throw new Error(`HTTP ${response.status}`);
		}
		const status = await response.json();
		console.log('  Transaction status:', status.group);
		if ('confirmed' === status.group) {
			console.log(`${label} confirmed in`, attempt, 'seconds');
			return;
		}
		if ('failed' === status.group)
			throw new Error(`${label} failed: ${status.code}`);
	}
	throw new Error(`${label} not confirmed after 60 seconds`);
}

// [>step-1]
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY)); // [<step-1]

const facade = new SymbolFacade('testnet');

try {
	// Fetch current network time [>step-2]
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log('  Network time:', timestamp.timestamp,
		'ms since nemesis'); // [<step-2]

	// Fetch recommended fees [>step-3]
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier); // [<step-3]

	// Build the transaction [>step-4]
	const transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: facade.network.publicKeyToAddress(
			signerKeyPair.publicKey).toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n // 1 XYM
		}]
	});
	transaction.fee = new models.Amount(
		calculateTransactionFee(transaction, feeMultiplier)); // [<step-4]

	// Sign transaction and generate final payload [>step-5]
	const signature = facade.signTransaction(signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true }); // [<step-5]

	// Announce the transaction [>step-6]
	await announceTransaction(jsonPayload, 'transaction'); // [<step-6]

	// Wait for confirmation [>step-7]
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);
	await waitForConfirmation(transactionHash, 'transaction'); // [<step-7]

} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
