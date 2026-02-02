import { PrivateKey } from 'symbol-sdk';
import {
	KeyPair,
	SymbolTransactionFactory,
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

// Returns the cosignatory addresses of the provided multisig account,
// or an empty list if the account is not multisig or has never been used
async function getMultisigCosignatories(address) {
	const multisigPath = `/account/${address}/multisig`;
	console.log(`Getting cosignatories from ${multisigPath}`);
	try {
		const response = await fetch(`${NODE_URL}${multisigPath}`);
		const json = await response.json();
		const cosignatories = json.multisig.cosignatoryAddresses;
		console.log('  Response:', JSON.stringify(cosignatories));
		return cosignatories;
	} catch {
		console.log('  Response: No cosignatories');
		return [];
	}
}

// Returns a transaction that turns a regular account into a multisig
function multisigEnableTransaction(timestamp, feeMult) {
	// Create an embedded multisig account modification transaction
	// that adds two cosignatories
	const embeddedTransaction = facade.transactionFactory
		.createEmbedded({
			type: 'multisig_account_modification_transaction_v1',
			// This is the account that will be turned into a multisig
			signerPublicKey: multisigKeyPair.publicKey,
			// Delta of the number of signatures required for approvals
			minApprovalDelta: 1,
			// Delta of the number of signatures required for removals
			minRemovalDelta: 1,
			addressAdditions: cosignatoryAddresses
		});

	// Build the aggregate transaction
	const embeddedTransactions = [embeddedTransaction];
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		// This is the account that will pay for this transaction
		signerPublicKey: multisigKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			embeddedTransactions),
		transactions: embeddedTransactions
	});
	// Reserve space for two cosignatures (each is 104 bytes)
	// and calculate fee for the final transaction size
	transaction.fee = new models.Amount(
		feeMult * (transaction.size + 104 * cosignatoryKeyPairs.length));
	console.log('Enabling the multisig with the aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));

	// Sign the aggregate transaction with the multisig's signature
	SymbolTransactionFactory.attachSignature(transaction,
		facade.signTransaction(multisigKeyPair, transaction));

	// Append signatures from all cosignatories
	for (const cosignatoryKeyPair of cosignatoryKeyPairs) {
		transaction.cosignatures.push(
			facade.cosignTransaction(cosignatoryKeyPair, transaction));
	}

	return transaction;
}

// Returns a transaction that turns a multisig into a regular account
function multisigDisableTransaction(timestamp, feeMult) {
	// Create two embedded multisig account modification transactions
	// because cosignatories must be removed one by one
	const embeddedTransaction1 = facade.transactionFactory
		.createEmbedded({
			type: 'multisig_account_modification_transaction_v1',
			// This is the multisig account that will be modified
			signerPublicKey: multisigKeyPair.publicKey,
			// Keep required signatures unchanged for this step
			minApprovalDelta: 0,
			minRemovalDelta: 0,
			addressDeletions: [cosignatoryAddresses[1]]
		});
	const embeddedTransaction2 = facade.transactionFactory
		.createEmbedded({
			type: 'multisig_account_modification_transaction_v1',
			// This is the multisig account that will be modified
			signerPublicKey: multisigKeyPair.publicKey,
			// Decrease required signatures after final removal
			minApprovalDelta: -1,
			minRemovalDelta: -1,
			addressDeletions: [cosignatoryAddresses[0]]
		});

	// Build the aggregate transaction
	const embeddedTransactions = [embeddedTransaction1,
		embeddedTransaction2];
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		// This is the account that will pay for this transaction
		signerPublicKey: cosignatoryKeyPairs[0].publicKey,
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			embeddedTransactions),
		transactions: embeddedTransactions
	});
	// Calculate fee for the final transaction size
	// (No need to reserve space for cosignatures, as there are none)
	transaction.fee = new models.Amount(feeMult * transaction.size);
	console.log('Disabling the multisig with the aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));

	// Sign the aggregate transaction using the first cosigner's signature
	SymbolTransactionFactory.attachSignature(transaction,
		facade.signTransaction(cosignatoryKeyPairs[0], transaction));

	return transaction;
}

const facade = new SymbolFacade('testnet');

const KEY_PREFIX = '0'.repeat(63);

// Setup the keys for the multisig account and its two cosignatories
const MULTISIG_PRIVATE_KEY = process.env.MULTISIG_PRIVATE_KEY || (
	KEY_PREFIX + '1');
const multisigKeyPair = new KeyPair(new PrivateKey(MULTISIG_PRIVATE_KEY));
const multisigAddress = facade.network.publicKeyToAddress(
	multisigKeyPair.publicKey);
console.log(`Multisig address: ${multisigAddress}`,
	`(public key ${multisigKeyPair.publicKey})`);

const cosignatoryKeyPairs = [];
const cosignatoryAddresses = [];
for (let i = 0; i < 2; i++) {
	const COSIGNATORY_PRIVATE_KEY =
		process.env[`COSIGNATORY${i}_PRIVATE_KEY`] || (
			KEY_PREFIX + String(i + 2));
	const kp = new KeyPair(new PrivateKey(COSIGNATORY_PRIVATE_KEY));
	cosignatoryKeyPairs.push(kp);
	const addr = facade.network.publicKeyToAddress(kp.publicKey);
	cosignatoryAddresses.push(addr);
	console.log(`Cosignatory ${i} address: ${addr}`,
		`(public key ${kp.publicKey})`);
}

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

	// Get current state of the multisig account and decide which
	// operation to perform
	const cosignatories = await getMultisigCosignatories(multisigAddress);
	let transaction;
	let signerKeyPair;
	if (cosignatories.length === 0) {
		// Enable the multisig
		transaction = multisigEnableTransaction(timestamp, feeMult);
		// This operation must be signed by the multisig account
		signerKeyPair = multisigKeyPair;
	} else {
		// Disable the multisig
		transaction = multisigDisableTransaction(timestamp, feeMult);
		// This operation must be signed by one of the cosigners
		signerKeyPair = cosignatoryKeyPairs[0];
	}
	const payload = SymbolTransactionFactory.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));

	// Announce and wait for confirmation
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log(
		'Built aggregate transaction with hash:', transactionHash);
	await announceTransaction(payload, 'aggregate transaction');
	await waitForConfirmation(transactionHash, 'aggregate transaction');

} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
