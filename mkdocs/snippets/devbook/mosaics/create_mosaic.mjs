import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
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
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier);
	// [<step-2]
	// --- CREATING MOSAIC DEFINITION ---
	console.log('\n--- Creating mosaic definition ---');
	// [>step-3]
	const nonce = Math.floor(Date.now() / 1000) & 0x7FFFFFFF;
	console.log('Mosaic nonce:', nonce);

	const definitionTx = facade.transactionFactory.create({
		type: 'mosaic_definition_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		duration: 0n,
		divisibility: 2,
		nonce,
		flags: 'transferable restrictable'
	});
	definitionTx.fee = new models.Amount(
		feeMultiplier * definitionTx.size);

	const mosaicId = generateMosaicId(signerAddress, nonce);
	console.log(`Mosaic ID: ${mosaicId} (0x${mosaicId.toString(16)})`);
	// [<step-3]
	// Sign and generate final payload [>step-4]
	const defSignature = facade.signTransaction(
		signerKeyPair, definitionTx);
	const defPayload = facade.transactionFactory.static.attachSignature(
		definitionTx, defSignature);
	console.log('Built mosaic definition transaction:');
	console.dir(definitionTx.toJson(), { colors: true });

	// Announce and wait for confirmation
	const definitionHash = facade.hashTransaction(definitionTx).toString();
	console.log('Transaction hash:', definitionHash);
	await announceTransaction(defPayload, 'mosaic definition');
	await waitForConfirmation(definitionHash, 'mosaic definition');
	// [<step-4]
	// --- INCREASING MOSAIC SUPPLY ---
	console.log('\n--- Increasing mosaic supply ---');
	// [>step-5]
	const supplyTx = facade.transactionFactory.create({
		type: 'mosaic_supply_change_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		mosaicId,
		action: 'increase',
		delta: 100_00n
	});
	supplyTx.fee = new models.Amount(feeMultiplier * supplyTx.size);
	// [<step-5]
	// Sign and generate final payload [>step-6]
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
	// [<step-6]
	// --- VERIFYING MOSAIC ---
	console.log('\n--- Verifying mosaic ---');
	// [>step-7]
	const mosaicIdHex = mosaicId.toString(16).padStart(16, '0');
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
	console.log('  Duration:', mosaicInfo.duration); // [<step-7]
} catch (e) {
	console.error(e.message);
}
