import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
	calculateTransactionFee,
	metadataGenerateKey,
	metadataUpdateValue,
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
		try {
			const response = await fetch(
				`${NODE_URL}/transactionStatus/${transactionHash}`);
			const status = await response.json();
			console.log('  Transaction status:', status.group);
			if ('confirmed' === status.group) {
				console.log(`${label} confirmed in`, attempt, 'seconds');
				return;
			}
			if ('failed' === status.group)
				throw new Error(`${label} failed: ${status.code}`);
		} catch (e) {
			if (e.message.includes('failed'))
				throw e;
			console.log('  Transaction status: unknown');
		}
	}
	throw new Error(`${label} not confirmed after 60 seconds`);
}
// [>step-1]
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());
// [<step-1]
try {
	// Fetch current network time [>step-2]
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
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier);
	// [<step-2]
	// --- ADDING NEW METADATA ---
	console.log('\n--- Adding new metadata ---');

	// Define metadata key and value [>step-3]
	const keyString = `username_${Date.now()}`;
	const scopedMetadataKey = metadataGenerateKey(keyString);
	const metadataValue = new TextEncoder().encode('alice');
	// [<step-3]
	// Create the embedded metadata transaction [>step-4]
	const creationEmbeddedTx = facade.transactionFactory
		.createEmbedded({
			type: 'account_metadata_transaction_v1',
			signerPublicKey: signerKeyPair.publicKey.toString(),
			targetAddress: signerAddress.toString(),
			scopedMetadataKey,
			// When creating new metadata, valueSizeDelta
			// equals value length
			valueSizeDelta: metadataValue.length,
			value: metadataValue
		});
	console.log('Created embedded metadata transaction:');
	console.log(JSON.stringify(creationEmbeddedTx.toJson(), null, 2));
	// [<step-4]
	// Build the aggregate transaction [>step-5]
	const creationEmbeddedTxs = [creationEmbeddedTx];
	const creationTx = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			creationEmbeddedTxs),
		transactions: creationEmbeddedTxs
	});
	creationTx.fee = new models.Amount(
		calculateTransactionFee(creationTx, feeMultiplier));
	// [<step-5]
	// Sign and generate final payload [>step-6]
	const signature = facade.signTransaction(signerKeyPair, creationTx);
	const creationPayload = facade.transactionFactory.static
		.attachSignature(creationTx, signature);

	// Announce and wait for confirmation
	const creationTxHash =
		facade.hashTransaction(creationTx).toString();
	console.log(
		'Built aggregate transaction with hash:', creationTxHash);
	await announceTransaction(creationPayload, 'creation transaction');
	await waitForConfirmation(creationTxHash, 'creation transaction');
	// [<step-6]
	// --- MODIFYING EXISTING METADATA ---
	console.log('\n--- Modifying existing metadata ---');

	// Fetch current metadata value from network [>step-7]
	const scopedKeyHex = scopedMetadataKey.toString(16)
		.toUpperCase().padStart(16, '0');
	const metadataPath = `/metadata?sourceAddress=${signerAddress}` +
		`&targetAddress=${signerAddress}` +
		`&scopedMetadataKey=${scopedKeyHex}` +
		'&metadataType=0';
	console.log('Fetching current metadata from', metadataPath);
	const metadataResponse = await fetch(`${NODE_URL}${metadataPath}`);
	const metadataJSON = await metadataResponse.json();

	// Get the metadata entry
	if (!metadataJSON.data.length)
		throw new Error('Metadata entry not found');

	const metadataEntry = metadataJSON.data[0].metadataEntry;
	const currentValue = Buffer.from(metadataEntry.value, 'hex');
	console.log('  Current value:', currentValue.toString('utf8'));
	// [<step-7]
	// XOR the current and new values [>step-8]
	const newValue = new TextEncoder().encode('bob');
	const updateValue = metadataUpdateValue(currentValue, newValue);

	// Create the update transaction with XOR'd value
	const updateEmbeddedTx = facade.transactionFactory
		.createEmbedded({
			type: 'account_metadata_transaction_v1',
			signerPublicKey: signerKeyPair.publicKey.toString(),
			targetAddress: signerAddress.toString(),
			scopedMetadataKey,
			// valueSizeDelta is the difference in length
			// (can be negative)
			valueSizeDelta: newValue.length - currentValue.length,
			value: updateValue
		});
	// [<step-8]
	// Build the aggregate for the update [>step-9]
	const updateEmbeddedTxs = [updateEmbeddedTx];
	const updateTx = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			updateEmbeddedTxs),
		transactions: updateEmbeddedTxs
	});
	updateTx.fee = new models.Amount(
		calculateTransactionFee(updateTx, feeMultiplier));

	// Sign and announce the update
	const updateSignature = facade.signTransaction(
		signerKeyPair, updateTx);
	const updatePayload = facade.transactionFactory.static
		.attachSignature(updateTx, updateSignature);

	// Announce and wait for confirmation
	const updateTxHash =
		facade.hashTransaction(updateTx).toString();
	console.log(
		'Built aggregate transaction with hash:', updateTxHash);
	await announceTransaction(updatePayload, 'update transaction');
	await waitForConfirmation(updateTxHash, 'update transaction');
	// [<step-9]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
