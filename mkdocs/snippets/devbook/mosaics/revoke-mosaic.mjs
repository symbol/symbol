import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Helper function to fetch account mosaic balances
async function getAccountMosaics(address) {
	const accountPath = `/accounts/${address}`;
	console.log('Fetching account information from', accountPath);
	const response = await fetch(`${NODE_URL}${accountPath}`);
	const responseJSON = await response.json();
	return responseJSON.account.mosaics;
}
// [>step-1]
const SIGNER_PRIVATE_KEY =
	process.env.SIGNER_PRIVATE_KEY
	|| '0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress =
	facade.network.publicKeyToAddress(signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

const SOURCE_ADDRESS = process.env.SOURCE_ADDRESS
	|| 'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA';
console.log('Source address:', SOURCE_ADDRESS);

const MOSAIC_ID_HEX = process.env.MOSAIC_ID
	|| '7aed3d514c986941';
const mosaicId = BigInt(`0x${MOSAIC_ID_HEX}`);
console.log(
	`Mosaic ID: ${mosaicId} (0x${MOSAIC_ID_HEX})`);
// [<step-1]
try {
	// Fetch current network time [>step-2]
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log('  Network time:',
		timestamp.timestamp, 'ms since nemesis');

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
	// --- CHECKING INITIAL BALANCE ---
	console.log('\n--- Checking initial balance ---');
	let mosaics = await getAccountMosaics(SOURCE_ADDRESS); // [>step-3]
	for (const mosaic of mosaics) {
		if (mosaic.id === MOSAIC_ID_HEX.toUpperCase())
			console.log(`  Mosaic ID: ${mosaic.id},`
				+ ` Amount: ${mosaic.amount}`);
	}
	// [<step-3]
	// --- REVOKING MOSAIC ---
	console.log('\n--- Revoking mosaic ---');
	// [>step-4]
	const revokeTx = facade.transactionFactory.create({
		type: 'mosaic_supply_revocation_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		sourceAddress: SOURCE_ADDRESS,
		mosaic: {
			mosaicId: mosaicId,
			amount: 7_00n
		}
	});
	revokeTx.fee = new models.Amount(feeMult * revokeTx.size);
	// [<step-4]
	// Sign and generate final payload [>step-5]
	const signature = facade.signTransaction(signerKeyPair, revokeTx);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
			revokeTx, signature);
	console.log('Built mosaic revocation transaction:');
	console.dir(revokeTx.toJson(), { colors: true });

	// Announce transaction
	const revokeHash = facade.hashTransaction(revokeTx).toString();
	console.log('Transaction hash:', revokeHash);

	console.log('Announcing mosaic revocation to /transactions');
	const announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());
	// [<step-5]
	// Wait for confirmation [>step-6]
	console.log('Waiting for mosaic revocation confirmation...');
	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const statusUrl =
				`${NODE_URL}/transactionStatus/${revokeHash}`;
			const statusResponse = await fetch(statusUrl);

			if (!statusResponse.ok) {
				console.log('  Transaction status: unknown');
				continue;
			}

			const status = await statusResponse.json();
			console.log('  Transaction status:', status.group);

			if (status.group === 'confirmed') {
				console.log('Mosaic revocation confirmed in',
					attempt, 'seconds');
				break;
			}

			if (status.group === 'failed') {
				throw new Error(
					'Mosaic revocation failed: ' + status.code);
			}
		} catch (error) {
			if (error.message.includes('failed:'))
				throw error;
			console.log('  Transaction status: unknown');
		}
	}
	// [<step-6]
	// --- VERIFYING REVOCATION ---
	console.log('\n--- Verifying revocation ---');
	mosaics = await getAccountMosaics(SOURCE_ADDRESS); // [>step-7]
	for (const mosaic of mosaics) {
		if (mosaic.id === MOSAIC_ID_HEX.toUpperCase())
			console.log(`  Mosaic ID: ${mosaic.id},`
				+ ` Amount: ${mosaic.amount}`);
	} // [<step-7]
} catch (e) {
	console.error(e.message);
}
