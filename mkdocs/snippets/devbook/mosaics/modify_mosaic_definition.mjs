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
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress =
	facade.network.publicKeyToAddress(signerKeyPair.publicKey);
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
	// Build the modification transaction [>step-3]
	const MOSAIC_NONCE = parseInt(process.env.MOSAIC_NONCE || '0', 10);
	console.log('Mosaic nonce:', MOSAIC_NONCE);

	const mosaicId = generateMosaicId(signerAddress, MOSAIC_NONCE);
	const mosaicIdHex = mosaicId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log(`Mosaic ID: ${mosaicId} (0x${mosaicIdHex})`);

	const modifyTx = facade.createTransactionFromTypedDescriptor(
		new descriptors.MosaicDefinitionTransactionV1Descriptor(
			new models.MosaicId(0n),
			new models.BlockDuration(0n),
			new models.MosaicNonce(MOSAIC_NONCE),
			models.MosaicFlags.REVOKABLE,
			0),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	// [<step-3]
	// Sign and generate final payload [>step-4]
	const signature = facade.signTransaction(
		signerKeyPair, modifyTx);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		modifyTx, signature);
	console.log('Built mosaic modification transaction:');
	console.dir(modifyTx.toJson(), { colors: true });

	const modifyHash = facade.hashTransaction(modifyTx).toString();
	console.log('Transaction hash:', modifyHash);

	// Announce transaction
	await announceTransaction(jsonPayload, 'mosaic modification');
	// [<step-4]
	// Wait for confirmation [>step-5]
	await waitForConfirmation(modifyHash, 'mosaic modification');
	// [<step-5]
	// Retrieve the mosaic [>step-6]
	const mosaicPath = `/mosaics/${mosaicIdHex}`;
	console.log('Fetching mosaic information from', mosaicPath);
	const mosaicResponse = await fetch(`${NODE_URL}${mosaicPath}`);
	const mosaicJSON = await mosaicResponse.json();
	const mosaicInfo = mosaicJSON.mosaic;
	console.log('Mosaic information:');
	console.log('  Mosaic ID:', mosaicInfo.id);
	console.log('  Supply:', mosaicInfo.supply);
	console.log('  Divisibility:', mosaicInfo.divisibility);
	console.log('  Flags:', mosaicInfo.flags);
	console.log('  Duration:', mosaicInfo.duration); // [<step-6]
} catch (e) {
	console.error(e.message);
}
