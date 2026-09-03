import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	descriptors,
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

// Helper function to fetch account mosaic balances
async function getAccountMosaics(address) {
	const accountPath = `/accounts/${address}`;
	console.log('Fetching account information from', accountPath);
	const response = await fetch(`${NODE_URL}${accountPath}`);
	const responseJSON = await response.json();
	return responseJSON.account.mosaics;
}
// [>step-1]
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress =
	facade.network.publicKeyToAddress(signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

const SOURCE_ADDRESS = process.env.SOURCE_ADDRESS ||
	'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA';
console.log('Source address:', SOURCE_ADDRESS);

const MOSAIC_ID_HEX = process.env.MOSAIC_ID ||
	'7AED3D514C986941';
const mosaicId = BigInt(`0x${MOSAIC_ID_HEX}`);
const mosaicIdHex = mosaicId.toString(16)
	.toUpperCase().padStart(16, '0');
console.log(
	`Mosaic ID: ${mosaicId} (0x${mosaicIdHex})`);
// [<step-1]
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
	// --- CHECKING INITIAL BALANCE ---
	console.log('\n--- Checking initial balance ---');
	let mosaics = await getAccountMosaics(SOURCE_ADDRESS); // [>step-3]
	for (const mosaic of mosaics) {
		if (mosaic.id === MOSAIC_ID_HEX.toUpperCase()) {
			console.log(`  Mosaic ID: ${mosaic.id},` +
				` Amount: ${mosaic.amount}`);
		}
	}
	// [<step-3]
	// --- REVOKING MOSAIC ---
	console.log('\n--- Revoking mosaic ---');
	// [>step-4]
	const revokeTx = facade.createTransactionFromTypedDescriptor(
		new descriptors.MosaicSupplyRevocationTransactionV1Descriptor(
			new SymbolFacade.Address(SOURCE_ADDRESS),
			new descriptors.UnresolvedMosaicDescriptor(
				new models.UnresolvedMosaicId(mosaicId),
				new models.Amount(7_00n))),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	// [<step-4]
	// Sign and generate final payload [>step-5]
	const signature = facade.signTransaction(signerKeyPair, revokeTx);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		revokeTx, signature);
	console.log('Built mosaic revocation transaction:');
	console.dir(revokeTx.toJson(), { colors: true });

	const revokeHash = facade.hashTransaction(revokeTx).toString();
	console.log('Transaction hash:', revokeHash);

	// Announce transaction
	await announceTransaction(jsonPayload, 'mosaic revocation');
	// [<step-5]
	// Wait for confirmation [>step-6]
	await waitForConfirmation(revokeHash, 'mosaic revocation');
	// [<step-6]
	// --- VERIFYING REVOCATION ---
	console.log('\n--- Verifying revocation ---');
	mosaics = await getAccountMosaics(SOURCE_ADDRESS); // [>step-7]
	for (const mosaic of mosaics) {
		if (mosaic.id === MOSAIC_ID_HEX.toUpperCase()) {
			console.log(`  Mosaic ID: ${mosaic.id},` +
				` Amount: ${mosaic.amount}`);
		}
	} // [<step-7]
} catch (e) {
	console.error(e.message);
}
