import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateMosaicId
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

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress =facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

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

	// --- CREATING MOSAIC DEFINITION ---
	console.log('\n--- Creating mosaic definition ---');

	const nonce = Math.floor(Date.now() / 1000) & 0x7FFFFFFF;
	console.log('Mosaic nonce:', nonce);

	const definitionTx =
		facade.transactionFactory.create({
			type: 'mosaic_definition_transaction_v1',
			signerPublicKey: signerKeyPair.publicKey.toString(),
			deadline: timestamp.addHours(2).timestamp,
			duration: 0n,
			divisibility: 2,
			nonce: nonce,
			flags: 'transferable restrictable'
		});
	definitionTx.fee = new models.Amount(feeMult * definitionTx.size);

	const mosaicId = generateMosaicId(signerAddress, nonce);
	console.log(`Mosaic ID: ${mosaicId} (0x${mosaicId.toString(16)})`);

	// Sign and generate final payload
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

	// --- INCREASING MOSAIC SUPPLY ---
	console.log('\n--- Increasing mosaic supply ---');

	const supplyTx =
		facade.transactionFactory.create({
			type: 'mosaic_supply_change_transaction_v1',
			signerPublicKey: signerKeyPair.publicKey.toString(),
			deadline: timestamp.addHours(2).timestamp,
			mosaicId: mosaicId,
			action: 'increase',
			delta: 100_00n
		});
	supplyTx.fee = new models.Amount(feeMult * supplyTx.size);

	// Sign and generate final payload
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

	// --- VERIFYING MOSAIC ---
	console.log('\n--- Verifying mosaic ---');

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
	console.log('  Duration:', mosaicInfo.duration);
} catch (e) {
	console.error(e.message);
}
