import { PrivateKey } from 'symbol-sdk';
import {
	KeyPair,
	SymbolFacade,
	descriptors
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const facade = new SymbolFacade('testnet');
// [>step-1]
const KEY_PREFIX = '0'.repeat(63);

// Setup the keys for the multisig account and its two cosignatories
const MULTISIG_PRIVATE_KEY = process.env.MULTISIG_PRIVATE_KEY || (
	`${KEY_PREFIX}1`);
const multisigKeyPair = new KeyPair(
	new PrivateKey(MULTISIG_PRIVATE_KEY));
const multisigAddress = facade.network.publicKeyToAddress(
	multisigKeyPair.publicKey);
console.log(`Multisig address: ${multisigAddress}`,
	`(public key ${multisigKeyPair.publicKey})`);

const cosignatoryKeyPairs = [];
const cosignatoryAddresses = [];
for (let i = 0; 2 > i; i++) {
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
// [<step-1]
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

// [>step-3]
// Returns the cosignatory addresses of the provided multisig account,
// or an empty list if the account is not multisig or has never been used
async function getMultisigCosignatories(address) {
	const multisigPath = `/account/${address}/multisig`;
	console.log(`Getting cosignatories from ${multisigPath}`);
	const response = await fetch(`${NODE_URL}${multisigPath}`);
	if (!response.ok) {
		console.log('  Response: No cosignatories');
		return [];
	}
	const json = await response.json();
	const cosignatories = json.multisig.cosignatoryAddresses;
	console.log('  Response:', JSON.stringify(cosignatories));
	return cosignatories;
}
// [<step-3]
// Returns a transaction that turns a regular account into a multisig
function multisigEnableTransaction(feeMultiplier) {
	// Create an embedded multisig account modification transaction [>step-5]
	// that adds two cosignatories
	const embeddedTransaction =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.MultisigAccountModificationTransactionV1Descriptor(
				// Delta of the number of signatures required for removals
				1,
				// Delta of the number of signatures required for approvals
				1,
				cosignatoryAddresses,
				undefined),
			multisigKeyPair.publicKey);
	// [<step-5]
	// Build the aggregate transaction [>step-6]
	const embeddedTransactions = [embeddedTransaction];
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(embeddedTransactions),
			embeddedTransactions,
			undefined),
		multisigKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60,
		cosignatoryKeyPairs.length);
	console.log('Enabling the multisig with the aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));
	// [<step-6]
	// Sign the aggregate transaction with the multisig's signature [>step-7]
	facade.transactionFactory.static.attachSignature(transaction,
		facade.signTransaction(multisigKeyPair, transaction));

	// Append signatures from all cosignatories
	for (const cosignatoryKeyPair of cosignatoryKeyPairs) {
		transaction.cosignatures.push(
			facade.cosignTransaction(cosignatoryKeyPair, transaction));
	}
	// [<step-7]
	return transaction;
}

// Returns a transaction that turns a multisig into a regular account
function multisigDisableTransaction(feeMultiplier) {
	// [>step-8]
	// Create two embedded multisig account modification transactions
	// because cosignatories must be removed one by one
	const embeddedTransaction1 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.MultisigAccountModificationTransactionV1Descriptor(
				// Keep required signatures unchanged for this step
				0,
				0,
				undefined,
				[cosignatoryAddresses[1]]),
			multisigKeyPair.publicKey);
	const embeddedTransaction2 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.MultisigAccountModificationTransactionV1Descriptor(
				// Decrease required signatures after final removal
				-1,
				-1,
				undefined,
				[cosignatoryAddresses[0]]),
			multisigKeyPair.publicKey);
	// [<step-8]
	// Build the aggregate transaction [>step-9]
	const embeddedTransactions = [embeddedTransaction1,
		embeddedTransaction2];
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(embeddedTransactions),
			embeddedTransactions,
			undefined),
		cosignatoryKeyPairs[0].publicKey,
		feeMultiplier,
		2 * 60 * 60);
	console.log(
		'Disabling the multisig with the aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));

	// Sign the aggregate with the first cosigner's signature
	facade.transactionFactory.static.attachSignature(transaction,
		facade.signTransaction(cosignatoryKeyPairs[0], transaction));
	// [<step-9]
	return transaction;
}

try {
	// Fetch recommended fees [>step-2]
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier);
	// [<step-2]
	// Get current state of the multisig account and decide which [>step-4]
	// operation to perform
	const cosignatories =
		await getMultisigCosignatories(multisigAddress);
	let transaction;
	if (0 === cosignatories.length) {
		// Enable the multisig
		transaction = multisigEnableTransaction(feeMultiplier);
	} else {
		// Disable the multisig
		transaction = multisigDisableTransaction(feeMultiplier);
	}
	const payload = facade.transactionFactory.static.toJson(transaction);
	// [<step-4]
	// Announce and wait for confirmation [>step-10]
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log(
		'Built aggregate transaction with hash:', transactionHash);
	await announceTransaction(payload, 'aggregate transaction');
	await waitForConfirmation(transactionHash, 'aggregate transaction');
	// [<step-10]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
