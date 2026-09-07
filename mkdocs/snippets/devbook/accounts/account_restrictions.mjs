import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	KeyPair,
	SymbolFacade,
	descriptors,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const facade = new SymbolFacade('testnet');
// [>step-1]
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new KeyPair(new PrivateKey(SIGNER_PRIVATE_KEY));
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log(`Signer address: ${signerAddress}`);

const authAddress = new Address('TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA');
console.log(`Authorized address: ${authAddress}`);
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
// Returns the list of restrictions currently applied to the account
async function getAccountRestrictions(address) {
	const restrictionsPath = `/restrictions/account/${address}`;
	console.log(`Getting restrictions from ${restrictionsPath}`);
	const response = await fetch(`${NODE_URL}${restrictionsPath}`);
	if (!response.ok) {
		console.log('  Response: No restrictions found');
		return [];
	}
	const json = await response.json();
	const restrictions = json.accountRestrictions.restrictions;
	console.log('  Response:', restrictions);
	return restrictions;
}
// [<step-3]
// Returns a transaction that restricts an account [>step-5]
function restrictionEnableTransaction(feeMultiplier) {
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AccountAddressRestrictionTransactionV1Descriptor(
			// Allow only OUTGOING transactions to the authorized ADDRESS
			models.AccountRestrictionFlags.ADDRESS.value |
				models.AccountRestrictionFlags.OUTGOING.value,
			// This is the only authorized outgoing address
			[authAddress],
			undefined),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	console.log('Enabling the restriction with transaction:');
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}
// [<step-5]
// Returns a transaction that removes a restriction from an account [>step-6]
function restrictionDisableTransaction(feeMultiplier, restriction) {
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AccountAddressRestrictionTransactionV1Descriptor(
			// Lift restrictions for OUTGOING ADDRESSES
			models.AccountRestrictionFlags.ADDRESS.value |
				models.AccountRestrictionFlags.OUTGOING.value,
			undefined,
			// Remove all addresses currently restricted
			restriction.values.map(hex =>
				Address.fromDecodedAddressHexString(hex))),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	console.log('Disabling the restriction with transaction:');
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}
// [<step-6]

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
	// Get current state of the restriction and decide which [>step-4]
	// operation to perform
	const restrictions = await getAccountRestrictions(signerAddress);
	let transaction;
	if (0 === restrictions.length) {
		// Enable the restriction
		console.log('\n--- Enabling restriction ---');
		transaction = restrictionEnableTransaction(feeMultiplier);
	} else {
		// Disable the restriction
		console.log('\n--- Disabling restriction ---');
		transaction = restrictionDisableTransaction(
			feeMultiplier, restrictions[0]);
	}
	// [<step-4]
	// Sign, announce and wait for confirmation [>step-7]
	let payload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));
	let hash = facade.hashTransaction(transaction).toString();
	await announceTransaction(payload, 'restriction transaction');
	await waitForConfirmation(hash, 'restriction transaction');
	// [<step-7]
	// Try a dummy transfer to a random address with no mosaics [>step-8]
	transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.TransferTransactionV1Descriptor(
			new Address('TBBHGE77IHHOIYA46B3XSORRNR2L5MLW54YO75Y'),
			undefined,
			undefined),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	payload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));
	hash = facade.hashTransaction(transaction).toString();
	console.log('\n--- Attempting transfer to unauthorized address ---');
	await announceTransaction(payload, 'test transfer');
	await waitForConfirmation(hash, 'test transfer');
	// [<step-8]
} catch (e) {
	console.error(e.message);
}
