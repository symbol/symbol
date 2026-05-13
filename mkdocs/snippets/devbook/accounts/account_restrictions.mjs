import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	KeyPair,
	NetworkTimestamp,
	SymbolFacade,
	SymbolTransactionFactory,
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

const authAddress = 'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA';
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

// Returns the list of restrictions currently applied to the account
async function getAccountRestrictions(address) { // [>step-3]
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
// Returns a transaction that restricts an account
function restrictionEnableTransaction(timestamp, feeMult) { // [>step-5]
	const transaction = facade.transactionFactory.create({
		type: 'account_address_restriction_transaction_v1',
		// This is the account that will be restricted
		signerPublicKey: signerKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		// Allow only OUTGOING transactions to the authorized ADDRESS
		restrictionFlags:
			models.AccountRestrictionFlags.ADDRESS.value |
			models.AccountRestrictionFlags.OUTGOING.value,
		// This is the only authorized outgoing address
		restrictionAdditions: [authAddress]
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	console.log('Enabling the restriction with transaction:');
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}
// [<step-5]
// Returns a transaction that removes a restriction from an account
function restrictionDisableTransaction(timestamp, feeMult, restriction) { // [>step-6]
	const transaction = facade.transactionFactory.create({
		type: 'account_address_restriction_transaction_v1',
		// This is the account whose restriction will be lifted
		signerPublicKey: signerKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		// Reverse flags
		restrictionFlags: restriction.restrictionFlags,
		// Remove all addresses currently restricted
		restrictionDeletions: restriction.values.map(hex =>
			Address.fromDecodedAddressHexString(hex))
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	console.log('Disabling the restriction with transaction:');
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}
// [<step-6]

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
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);
	// [<step-2]
	// Get current state of the restriction and decide which
	// operation to perform
	const restrictions = await getAccountRestrictions(signerAddress); // [>step-4]
	let transaction;
	if (0 === restrictions.length) {
		// Enable the restriction
		console.log('\n--- Enabling restriction ---');
		transaction = restrictionEnableTransaction(timestamp, feeMult);
	} else {
		// Disable the restriction
		console.log('\n--- Disabling restriction ---');
		transaction = restrictionDisableTransaction(timestamp, feeMult,
			restrictions[0]);
	}
	// [<step-4]
	// Sign, announce and wait for confirmation [>step-7]
	let payload = SymbolTransactionFactory.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));
	let hash = facade.hashTransaction(transaction).toString();
	await announceTransaction(payload, 'restriction transaction');
	await waitForConfirmation(hash, 'restriction transaction');
	// [<step-7]
	// Try a dummy transfer to a random address with no mosaics [>step-8]
	transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: 'TBBHGE77IHHOIYA46B3XSORRNR2L5MLW54YO75Y'
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	payload = SymbolTransactionFactory.attachSignature(
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
