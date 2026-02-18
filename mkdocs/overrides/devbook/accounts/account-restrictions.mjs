import { PrivateKey } from 'symbol-sdk';
import {
	KeyPair,
	SymbolTransactionFactory,
	models,
	Address,
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

// Returns the list of restrictions currently applied to the account
async function getAccountRestrictions(address) {
	const restrictionsPath = `/restrictions/account/${address}`;
	console.log(`Getting restrictions from ${restrictionsPath}`);
	try {
		const response = await fetch(`${NODE_URL}${restrictionsPath}`);
		const json = await response.json();
		const restrictions = json.accountRestrictions.restrictions;
		console.log('  Response:', restrictions);
		return restrictions;
	} catch {
		console.log('  Response: No restrictions found');
		return [];
	}
}

// Returns a transaction that restricts an account
function restrictionEnableTransaction(timestamp, feeMult) {
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

// Returns a transaction that removes a restriction from an account
function restrictionDisableTransaction(timestamp, feeMult, restriction) {
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

const facade = new SymbolFacade('testnet');

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new KeyPair(new PrivateKey(SIGNER_PRIVATE_KEY));
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log(`Signer address: ${signerAddress}`);

const AUTH_PRIVATE_KEY = process.env.AUTH_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000001';
const authKeyPair = new KeyPair(new PrivateKey(AUTH_PRIVATE_KEY));
const authAddress = facade.network.publicKeyToAddress(
	authKeyPair.publicKey);
console.log(`Authorized address: ${authAddress}`);

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

	// Get current state of the restriction and decide which
	// operation to perform
	const restrictions = await getAccountRestrictions(signerAddress);
	let transaction;
	if (restrictions.length === 0) {
		// Enable the restriction
		console.log('\n--- Enabling restriction ---');
		transaction = restrictionEnableTransaction(timestamp, feeMult);
	} else {
		// Disable the restriction
		console.log('\n--- Disabling restriction ---');
		transaction = restrictionDisableTransaction(timestamp, feeMult, restrictions[0]);
	}
	let payload = SymbolTransactionFactory.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));

	// Announce and wait for confirmation
	let hash =
		facade.hashTransaction(transaction).toString();
	await announceTransaction(payload, 'restriction transaction');
	await waitForConfirmation(hash, 'restriction transaction');

	// Try a dummy transfer to a random address with no mosaics
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

} catch (e) {
	console.error(e.message);
}
