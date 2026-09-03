import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	descriptors,
	generateMosaicId,
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
	// --- CREATING MOSAIC DEFINITION ---
	console.log('\n--- Creating mosaic definition ---');
	// [>step-3]
	const nonce = Math.floor(Date.now() / 1000) % 0x100000000;
	console.log('Mosaic nonce:', nonce);
	// [<step-3]
	// Build the mosaic definition transaction [>step-4]
	const definitionTx = facade.createTransactionFromTypedDescriptor(
		new descriptors.MosaicDefinitionTransactionV1Descriptor(
			new models.MosaicId(0n),
			new models.BlockDuration(0n),
			new models.MosaicNonce(nonce),
			new models.MosaicFlags(
				models.MosaicFlags.TRANSFERABLE.value |
				models.MosaicFlags.RESTRICTABLE.value),
			2),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);

	const mosaicId = generateMosaicId(signerAddress, nonce);
	const mosaicIdHex = mosaicId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log(`Mosaic ID: ${mosaicId} (0x${mosaicIdHex})`);
	// [<step-4]
	// Sign and generate final payload [>step-5]
	const defSignature = facade.signTransaction(
		signerKeyPair, definitionTx);
	const defPayload = facade.transactionFactory.static.attachSignature(
		definitionTx, defSignature);
	console.log('Built mosaic definition transaction:');
	console.dir(definitionTx.toJson(), { colors: true });

	// Announce and wait for confirmation
	const definitionHash =
		facade.hashTransaction(definitionTx).toString();
	console.log('Transaction hash:', definitionHash);
	await announceTransaction(defPayload, 'mosaic definition');
	await waitForConfirmation(definitionHash, 'mosaic definition');
	// [<step-5]
	// --- INCREASING MOSAIC SUPPLY ---
	console.log('\n--- Increasing mosaic supply ---');
	// [>step-6]
	const supplyTx = facade.createTransactionFromTypedDescriptor(
		new descriptors.MosaicSupplyChangeTransactionV1Descriptor(
			new models.UnresolvedMosaicId(mosaicId),
			new models.Amount(100_00n),
			models.MosaicSupplyChangeAction.INCREASE),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	// [<step-6]
	// Sign and generate final payload [>step-7]
	const supSignature = facade.signTransaction(
		signerKeyPair, supplyTx);
	const supPayload = facade.transactionFactory.static.attachSignature(
		supplyTx, supSignature);
	console.log('Built mosaic supply change transaction:');
	console.dir(supplyTx.toJson(), { colors: true });

	// Announce and wait for confirmation
	const supplyHash = facade.hashTransaction(supplyTx).toString();
	console.log('Transaction hash:', supplyHash);
	await announceTransaction(supPayload, 'mosaic supply change');
	await waitForConfirmation(supplyHash, 'mosaic supply change');
	// [<step-7]
	// --- VERIFYING MOSAIC ---
	console.log('\n--- Verifying mosaic ---');
	// [>step-8]
	const mosaicPath = `/mosaics/${mosaicIdHex}`;
	console.log('Fetching mosaic information from', mosaicPath);
	const mosaicResponse = await fetch(`${NODE_URL}${mosaicPath}`);
	const mosaicJSON = await mosaicResponse.json();
	const mosaicInfo = mosaicJSON.mosaic;
	console.log('Mosaic information:');
	console.log('  Mosaic ID:', mosaicInfo.id);
	console.log('  Supply:', mosaicInfo.supply);
	console.log('  Flags:', mosaicInfo.flags);
	console.log('  Divisibility:', mosaicInfo.divisibility);
	console.log('  Duration:', mosaicInfo.duration); // [<step-8]
} catch (e) {
	console.error(e.message);
}
