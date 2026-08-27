import { PrivateKey } from 'symbol-sdk';
import {
	KeyPair,
	SymbolFacade,
	SymbolTransactionFactory,
	descriptors,
	models,
	mosaicRestrictionGenerateKey
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const facade = new SymbolFacade('testnet');
// [>step-1]
const OWNER_PRIVATE_KEY = process.env.OWNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const ownerKeyPair = new KeyPair(new PrivateKey(OWNER_PRIVATE_KEY));
const ownerAddress = facade.network.publicKeyToAddress(
	ownerKeyPair.publicKey);
console.log(`Owner address: ${ownerAddress}`);

const targetAddress = process.env.TARGET_ADDRESS ||
	'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA';
console.log(`Target address: ${targetAddress}`);

const mosaicId = BigInt(`0x${process.env.MOSAIC_ID ||
	'6A5ACF2376E50D4A'}`);
console.log(`Mosaic ID: 0x${mosaicId.toString(16)
	.toUpperCase().padStart(16, '0')}`);

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

// Returns restrictions currently applied to the mosaic
// matching the given restriction key
async function getMosaicRestrictions(query, key) { // [>step-4]
	const restrictionsPath = `/restrictions/mosaic?${query}`;
	console.log(`  Getting restrictions from ${restrictionsPath}`);
	let res = [];
	const response = await fetch(`${NODE_URL}${restrictionsPath}`);
	const status = await response.json();
	const data = status.data;
	if (0 < data.length) {
		// Look at the first returned restriction
		const rlist = data[0].mosaicRestrictionEntry.restrictions;
		// Filter by key
		res = rlist.filter(r => BigInt(r.key) === key);
	}
	console.log('  Response:', res);
	return res;
}

function getMosaicGlobalRestrictions(queriedMosaicId, key) {
	return getMosaicRestrictions(
		`mosaicId=${queriedMosaicId.toString(16)
			.toUpperCase().padStart(16, '0')}` +
		'&entryType=1', key);
}
// [<step-4] [>step-5]
function getMosaicAddressRestrictions(queriedMosaicId, address, key) {
	return getMosaicRestrictions(
		`mosaicId=${queriedMosaicId.toString(16)
			.toUpperCase().padStart(16, '0')}` +
		`&entryType=0&targetAddress=${address}`, key);
}
// [<step-5]
// Returns a transaction enabling a mosaic's global restriction
function globalRestrictionEnableTransaction() {
	const transaction = facade.createEmbeddedTransactionFromTypedDescriptor(
		new descriptors.MosaicGlobalRestrictionTransactionV1Descriptor(
			new models.UnresolvedMosaicId(mosaicId),
			new models.UnresolvedMosaicId(0n),
			restrictionKey,
			0n,
			1n,
			models.MosaicRestrictionType.NONE,
			models.MosaicRestrictionType.GE),
		ownerKeyPair.publicKey);
	console.dir(transaction.toJson(), { colors: true, depth: null });

	return transaction;
}

// Returns a transaction setting an address restriction's value
function addressRestrictionSetValue(prevValue, newValue, address) {
	const transaction = facade.createEmbeddedTransactionFromTypedDescriptor(
		new descriptors.MosaicAddressRestrictionTransactionV1Descriptor(
			new models.UnresolvedMosaicId(mosaicId),
			restrictionKey,
			prevValue,
			newValue,
			new SymbolFacade.Address(address)),
		ownerKeyPair.publicKey);
	console.dir(transaction.toJson(), { colors: true, depth: null });

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
	const aggregate = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(transactions),
			transactions,
			undefined),
		ownerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
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
	const transfer = facade.createTransactionFromTypedDescriptor( // [>step-9]
		new descriptors.TransferTransactionV1Descriptor(
			new SymbolFacade.Address(targetAddress),
			[
				new descriptors.UnresolvedMosaicDescriptor(
					new models.UnresolvedMosaicId(mosaicId),
					new models.Amount(1n))
			],
			undefined),
		ownerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);

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
