import { PrivateKey } from 'symbol-sdk';
import {
	KeyPair,
	NetworkTimestamp,
	SymbolFacade,
	SymbolTransactionFactory,
	models,
	mosaicRestrictionGenerateKey
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const facade = new SymbolFacade('testnet');
// [>step-1]
const OWNER_PRIVATE_KEY = process.env.OWNER_PRIVATE_KEY
	|| '0000000000000000000000000000000000000000000000000000000000000000';
const ownerKeyPair = new KeyPair(new PrivateKey(OWNER_PRIVATE_KEY));
const ownerAddress = facade.network.publicKeyToAddress(
	ownerKeyPair.publicKey);
console.log(`Owner address: ${ownerAddress}`);

const targetAddress = process.env.TARGET_ADDRESS
	|| 'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA';
console.log(`Target address: ${targetAddress}`);

const mosaicId = BigInt(`0x${process.env.MOSAIC_ID
	|| '6A5ACF2376E50D4A'}`);
console.log(`Mosaic ID: 0x${mosaicId.toString(16).toUpperCase()}`);

const restrictionName = process.env.RESTRICTION_NAME || 'security_level';
const restrictionKey = mosaicRestrictionGenerateKey(restrictionName);
console.log(`Restriction name: "${restrictionName}" (key: 0x${
	restrictionKey.toString(16).toUpperCase().padStart(16, '0')})`);
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

// Returns a filtered list of restrictions currently applied to the mosaic
// matching the given restriction key
async function getMosaicRestrictions(query, key) { // [>step-4]
	const restrictionsPath = `/restrictions/mosaic?${query}`;
	console.log(`  Getting restrictions from ${restrictionsPath}`);
	let res = [];
	try {
		const response = await fetch(`${NODE_URL}${restrictionsPath}`);
		const status = await response.json();
		const data = status.data;
		if (0 < data.length) {
			// Look at the first returned restriction
			const rlist = data[0].mosaicRestrictionEntry.restrictions;
			// Filter by key
			res = rlist.filter(r => BigInt(r.key) === key);
		}
	} catch {
		// The mosaic has no restrictions applied to this key
	}
	console.log('  Response:', res);
	return res;
}

function getMosaicGlobalRestrictions(queriedMosaicId, key) {
	return getMosaicRestrictions(
		`mosaicId=${queriedMosaicId.toString(16)}&entryType=1`,
		key);
}
// [<step-4] [>step-5]
function getMosaicAddressRestrictions(queriedMosaicId, address, key) {
	return getMosaicRestrictions(
		`mosaicId=${queriedMosaicId.toString(16)}&entryType=0`
		+ `&targetAddress=${address}`, key);
}
// [<step-5]
// Returns a transaction enabling a mosaic's global restriction
function globalRestrictionEnableTransaction() {
	const transaction = facade.transactionFactory.createEmbedded({
		type: 'mosaic_global_restriction_transaction_v1',
		signerPublicKey: ownerKeyPair.publicKey,
		mosaicId,
		referenceMosaicId: 0n,
		restrictionKey,
		previousRestrictionType: 0,
		previousRestrictionValue: 0n,
		newRestrictionType: 'ge',
		newRestrictionValue: 1n
	});
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}

// Returns a transaction setting an address restriction's value
function addressRestrictionSetValue(prevValue, newValue, address) {
	const transaction = facade.transactionFactory.createEmbedded({
		type: 'mosaic_address_restriction_transaction_v1',
		signerPublicKey: ownerKeyPair.publicKey,
		mosaicId,
		restrictionKey,
		previousRestrictionValue: prevValue,
		newRestrictionValue: newValue,
		targetAddress: address
	});
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}

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
	// Enable global restriction if required [>step-3]
	const transactions = [];
	console.log('Checking if the global restriction is enabled:');
	const globalRestrictions = await getMosaicGlobalRestrictions(
		mosaicId, restrictionKey);
	if (0 === globalRestrictions.length) {
		// Enable the global restriction
		console.log('+ Enabling global restriction');
		transactions.push(globalRestrictionEnableTransaction());

		// Enable the address restriction
		console.log('+ Authorizing owner account');
		transactions.push(addressRestrictionSetValue(
			0xFFFFFFFFFFFFFFFFn, 1n, ownerAddress.toString()));
	}
	// [<step-3]
	// Toggle target address restriction
	console.log('Checking if target account is authorized:'); // [>step-6]
	const addressRestrictions = await getMosaicAddressRestrictions(
		mosaicId, targetAddress, restrictionKey);
	let prevValue = 0xFFFFFFFFFFFFFFFFn;
	if (0 < addressRestrictions.length)
		prevValue = BigInt(addressRestrictions[0].value);
	if (1n !== prevValue) {
		// Enable the address restriction
		console.log('+ Authorizing target account');
		transactions.push(addressRestrictionSetValue(
			prevValue, 1n, targetAddress));
	} else {
		// Disable the address restriction
		console.log('+ Deauthorizing target account');
		transactions.push(addressRestrictionSetValue(
			prevValue, 0n, targetAddress));
	}
	// [<step-6]
	// Build an aggregate transaction
	console.log('Bundling', transactions.length, // [>step-7]
		'transaction(s) in an aggregate');
	const aggregate = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: ownerKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			transactions),
		transactions
	});
	aggregate.fee = new models.Amount(feeMult * aggregate.size);
	// [<step-7]
	// Sign, announce and wait for confirmation
	let payload = SymbolTransactionFactory.attachSignature( // [>step-8]
		aggregate,
		facade.signTransaction(ownerKeyPair, aggregate));
	let hash = facade.hashTransaction(aggregate).toString();
	await announceTransaction(payload, 'aggregate');
	await waitForConfirmation(hash, 'aggregate');
	// [<step-8]
	// Try to transfer the mosaic to the target address
	const transfer = facade.transactionFactory.create({ // [>step-9]
		type: 'transfer_transaction_v1',
		signerPublicKey: ownerKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: targetAddress,
		mosaics: [{
			mosaicId,
			amount: 1n
		}]
	});
	transfer.fee = new models.Amount(feeMult * transfer.size);

	payload = SymbolTransactionFactory.attachSignature(
		transfer,
		facade.signTransaction(ownerKeyPair, transfer));
	hash = facade.hashTransaction(transfer).toString();
	console.log('\nAttempting transfer to the target account');
	await announceTransaction(payload, 'test transfer');
	await waitForConfirmation(hash, 'test transfer');
	// [<step-9]
} catch (e) {
	console.error(e.message);
}
