import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateMosaicId
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY
	|| '0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress =
	facade.network.publicKeyToAddress(signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

try {
	// Fetch current network time
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log(
		'  Network time:', timestamp.timestamp, 'ms since nemesis');

	// Fetch recommended fees
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);

	// Build the modification transaction
	const MOSAIC_NONCE = parseInt(process.env.MOSAIC_NONCE || '0', 10);
	console.log('Mosaic nonce:', MOSAIC_NONCE);

	const mosaicId = generateMosaicId(signerAddress, MOSAIC_NONCE);
	console.log(
		`Mosaic ID: ${mosaicId} (0x${mosaicId.toString(16)})`);

	const modifyTx = facade.transactionFactory.create({
		type: 'mosaic_definition_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		duration: 0n,
		divisibility: 0,
		nonce: MOSAIC_NONCE,
		flags: 'revokable'
	});
	modifyTx.fee = new models.Amount(feeMult * modifyTx.size);

	// Sign and generate final payload
	const signature = facade.signTransaction(
		signerKeyPair, modifyTx);
	const jsonPayload =
		facade.transactionFactory.static.attachSignature(
			modifyTx, signature);
	console.log('Built mosaic modification transaction:');
	console.dir(modifyTx.toJson(), { colors: true });

	const modifyHash =
		facade.hashTransaction(modifyTx).toString();
	console.log('Transaction hash:', modifyHash);

	// Announce transaction
	console.log(
		'Announcing mosaic modification to /transactions');
	const announceResponse = await fetch(
		`${NODE_URL}/transactions`, {
			method: 'PUT',
			headers: { 'Content-Type': 'application/json' },
			body: jsonPayload
		});
	console.log('  Response:', await announceResponse.text());

	// Wait for confirmation
	console.log(
		'Waiting for mosaic modification confirmation...');
	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));

		try {
			const statusUrl =
				`${NODE_URL}/transactionStatus/${modifyHash}`;
			const statusResponse = await fetch(statusUrl);

			if (!statusResponse.ok) {
				console.log('  Transaction status: unknown');
				continue;
			}

			const status = await statusResponse.json();
			console.log('  Transaction status:', status.group);

			if (status.group === 'confirmed') {
				console.log(
					'Mosaic modification confirmed in',
					attempt, 'seconds');
				break;
			}

			if (status.group === 'failed') {
				throw new Error(
					'Mosaic modification failed:'
					+ ` ${status.code}`);
			}
		} catch (error) {
			if (error.message.includes('failed:')) {
				throw error;
			}
			console.log('  Transaction status: unknown');
		}
	}

	// Retrieve the mosaic
	const mosaicIdHex = mosaicId.toString(16);
	const mosaicPath = `/mosaics/${mosaicIdHex}`;
	console.log('Fetching mosaic information from', mosaicPath);
	const mosaicResponse =
		await fetch(`${NODE_URL}${mosaicPath}`);
	const mosaicJSON = await mosaicResponse.json();
	const mosaicInfo = mosaicJSON.mosaic;
	console.log('Mosaic information:');
	console.log('  Mosaic ID:', mosaicInfo.id);
	console.log('  Supply:', mosaicInfo.supply);
	console.log('  Divisibility:', mosaicInfo.divisibility);
	console.log('  Flags:', mosaicInfo.flags);
	console.log('  Duration:', mosaicInfo.duration);
} catch (e) {
	console.error(e.message);
}
