import { PrivateKey } from 'symbol-sdk';
import {
	generateMosaicAliasId,
	models,
	NetworkTimestamp,
	SymbolFacade
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://001-sai-dual.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Account A (initiates the aggregate and sends XYM)
const ACCOUNT_A_PRIVATE_KEY = process.env.ACCOUNT_A_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const accountAKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_A_PRIVATE_KEY));

// Account B (sends custom mosaic back to Account A)
const ACCOUNT_B_PRIVATE_KEY = process.env.ACCOUNT_B_PRIVATE_KEY || (
	'1111111111111111111111111111111111111111111111111111111111111111');
const accountBKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_B_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
console.log('Account A:', facade.network.publicKeyToAddress(
	accountAKeyPair.publicKey).toString());
console.log('Account B:', facade.network.publicKeyToAddress(
	accountBKeyPair.publicKey).toString());

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

	const accountAAddress = facade.network.publicKeyToAddress(
		accountAKeyPair.publicKey);
	const accountBAddress = facade.network.publicKeyToAddress(
		accountBKeyPair.publicKey);

	// Account A sends 10 XYM to Account B
	const embeddedTransaction1 = facade.transactionFactory
		.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		recipientAddress: accountBAddress.toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 10_000_000n  // 10 XYM (divisibility = 6)
		}]
	});

	// Account B sends 1 custom token to Account A
	const customMosaicId = 0x6D1314BE751B62C2n;
	const embeddedTransaction2 = facade.transactionFactory
		.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: accountBKeyPair.publicKey.toString(),
		recipientAddress: accountAAddress.toString(),
		mosaics: [{
			mosaicId: customMosaicId,
			amount: 1n  // 1 custom mosaic (divisibility = 0)
		}]
	});

	// Build the aggregate transaction
	const embeddedTransactions = [
		embeddedTransaction1, embeddedTransaction2];
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			embeddedTransactions),
		transactions: embeddedTransactions
	});
	// Reserve space for one cosignature (104 bytes each)
	// and calculate fee for the final transaction size
	transaction.fee = new models.Amount(
		feeMult * (transaction.size + 104)
	);
	console.log('Built aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));

	// --- ACCOUNT A (Initiator) ---
	console.log('[Account A] Signing the aggregate...');
	const signatureA = facade.signTransaction(accountAKeyPair, transaction);
	const transactionPayload = facade.transactionFactory.static
		.attachSignature(transaction, signatureA);
	const payloadFormatted = JSON.stringify(
		JSON.parse(transactionPayload), null, 2);
	console.log('[Account A] Payload ready to share:\n', payloadFormatted);

	// --- OFF-CHAIN COORDINATION ---
	// Account A sends the payload to Account B
	const sharedPayload = transactionPayload;
	console.log('[Sharing] Payload sent to Account B...');

	// --- ACCOUNT B (Cosignatory) ---
	console.log('[Account B] Received payload');
	const receivedTransaction = facade.transactionFactory.static
		.deserialize(Buffer.from(JSON.parse(sharedPayload).payload, 'hex'));

	console.log('[Account B] Cosigning...');
	const cosignatureB = facade.cosignTransaction(
		accountBKeyPair, receivedTransaction);
	console.log('[Account B] Cosignature created:', cosignatureB.toString());

	// --- OFF-CHAIN COORDINATION ---
	// Account B sends the cosignature back to Account A
	const sharedCosignature = cosignatureB;
	console.log('[Sharing] Cosignature sent back to Account A...');

	// --- ACCOUNT A (Initiator) ---
	console.log('[Account A] Received cosignature');
	// Add cosignature to the transaction and rebuild payload
	transaction.cosignatures.push(sharedCosignature);
	const transactionPayloadFinal = facade.transactionFactory.static
		.attachSignature(transaction, signatureA);
	const jsonPayload = transactionPayloadFinal;
	console.log('[Account A] Ready to announce');

	// Announce the transaction
	const announcePath = '/transactions';
	console.log('Announcing transaction to', announcePath);
	const announceResponse = await fetch(`${NODE_URL}${announcePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());

	// Compute hash of final transaction (with cosignatures)
	const transactionHash =
		facade.hashTransaction(transaction).toString();

	// Wait for confirmation
	const statusPath = `/transactionStatus/${transactionHash}`;
	console.log('Waiting for confirmation from', statusPath);

	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const statusResponse = await fetch(`${NODE_URL}${statusPath}`);
			const status = await statusResponse.json();
			console.log('  Transaction status:', status.group);
			if (status.group === 'confirmed') {
				console.log('Transaction confirmed in', attempt, 'seconds');
				break;
			}
			if (status.group === 'failed') {
				console.log('Transaction failed:', status.code);
				break;
			}
		} catch (e) {
			console.log('  Transaction status: unknown | Cause:',
				e.message);
		}
	}
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
