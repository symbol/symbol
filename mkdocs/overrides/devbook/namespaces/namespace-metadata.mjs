import { PrivateKey } from 'symbol-sdk';
import {
	generateNamespaceId,
	metadataGenerateKey,
	metadataUpdateValue,
	models,
	NetworkTimestamp,
	SymbolFacade
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
	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const response = await fetch(
				`${NODE_URL}/transactionStatus/${transactionHash}`);
			const status = await response.json();
			console.log('  Transaction status:', status.group);
			if (status.group === 'confirmed') {
				console.log(`${label} confirmed in`, attempt, 'seconds');
				return;
			}
			if (status.group === 'failed') {
				throw new Error(`${label} failed: ${status.code}`);
			}
		} catch (e) {
			if (e.message.includes('failed'))
				throw e;
			console.log('  Transaction status: unknown');
		}
	}
	throw new Error(`${label} not confirmed after 60 seconds`);
}

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

// Get namespace name from environment or use default
const NAMESPACE_NAME = process.env.NAMESPACE_NAME || 'testnamespace';
const namespaceId = generateNamespaceId(NAMESPACE_NAME);
console.log('Namespace name:', NAMESPACE_NAME);
console.log('Namespace ID:',
	namespaceId.toString(), `(0x${namespaceId.toString(16)})`);

try {
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
	console.log('  Fee multiplier:', feeMult);

	// --- ADDING NEW METADATA ---
	console.log('\n--- Adding new metadata ---');

	// Define metadata key and value
	const keyString = `description_${Date.now()}`;
	const scopedMetadataKey = metadataGenerateKey(keyString);
	const metadataValue = new TextEncoder().encode('My first namespace');

	// Create the embedded metadata transaction
	const embeddedTransaction = facade.transactionFactory
		.createEmbedded({
			type: 'namespace_metadata_transaction_v1',
			signerPublicKey: signerKeyPair.publicKey.toString(),
			targetAddress: signerAddress.toString(),
			targetNamespaceId: namespaceId,
			scopedMetadataKey,
			// When creating new metadata, valueSizeDelta
			// equals value length
			valueSizeDelta: metadataValue.length,
			value: metadataValue
		});
	console.log('Created embedded metadata transaction:');
	console.log(JSON.stringify(embeddedTransaction.toJson(), null, 2));

	// Build the aggregate transaction
	const embeddedTransactions = [embeddedTransaction];
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			embeddedTransactions),
		transactions: embeddedTransactions
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);

	// Sign and generate final payload
	const signature = facade.signTransaction(signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);

	// Announce and wait for confirmation
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log(
		'Built aggregate transaction with hash:', transactionHash);
	await announceTransaction(jsonPayload, 'aggregate transaction');
	await waitForConfirmation(transactionHash, 'aggregate transaction');

	// --- MODIFYING EXISTING METADATA ---
	console.log('\n--- Modifying existing metadata ---');

	// Fetch current metadata value from network
	const scopedKeyHex = scopedMetadataKey.toString(16)
		.toUpperCase().padStart(16, '0');
	const namespaceIdHex = namespaceId.toString(16)
		.toUpperCase().padStart(16, '0');
	const metadataPath = `/metadata`
		+ `?targetAddress=${signerAddress}`
		+ `&scopedMetadataKey=${scopedKeyHex}`
		+ `&targetId=${namespaceIdHex}`
		+ '&metadataType=2';
	console.log('Fetching current metadata from', metadataPath);
	const metadataResponse = await fetch(`${NODE_URL}${metadataPath}`);
	const metadataJSON = await metadataResponse.json();

	// Get the metadata entry
	if (!metadataJSON.data.length) {
		throw new Error('Metadata entry not found');
	}
	const metadataEntry = metadataJSON.data[0].metadataEntry;
	const currentValue = Buffer.from(metadataEntry.value, 'hex');
	console.log('  Current value:', currentValue.toString('utf8'));

	// XOR the current and new values
	const newValue = new TextEncoder().encode('Updated namespace');
	const updateValue = metadataUpdateValue(currentValue, newValue);

	// Create the update transaction with XOR'd value
	const embeddedUpdate = facade.transactionFactory
		.createEmbedded({
			type: 'namespace_metadata_transaction_v1',
			signerPublicKey: signerKeyPair.publicKey.toString(),
			targetAddress: signerAddress.toString(),
			targetNamespaceId: namespaceId,
			scopedMetadataKey,
			// valueSizeDelta is the difference in length
			// (can be negative)
			valueSizeDelta: newValue.length - currentValue.length,
			value: updateValue
		});
	console.log('Created embedded update transaction:');
	console.log(JSON.stringify(embeddedUpdate.toJson(), null, 2));

	// Build the aggregate for the update
	const updateEmbedded = [embeddedUpdate];
	const updateTransaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			updateEmbedded),
		transactions: updateEmbedded
	});
	updateTransaction.fee = new models.Amount(
		feeMult * updateTransaction.size);

	// Sign and announce the update
	const updateSignature = facade.signTransaction(
		signerKeyPair, updateTransaction);
	const updatePayload = facade.transactionFactory.static
		.attachSignature(updateTransaction, updateSignature);

	// Announce and wait for confirmation
	const updateHash =
		facade.hashTransaction(updateTransaction).toString();
	console.log(
		'Built aggregate transaction with hash:', updateHash);
	await announceTransaction(updatePayload, 'aggregate transaction');
	await waitForConfirmation(updateHash, 'aggregate transaction');

} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
