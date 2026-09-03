import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	descriptors,
	generateMosaicAliasId,
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
// Account A (initiates the aggregate tx and sends XYM to Account B)
const ACCOUNT_A_PRIVATE_KEY = process.env.ACCOUNT_A_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const accountAKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_A_PRIVATE_KEY));

// Account B (sends custom mosaic to Account A)
const ACCOUNT_B_PRIVATE_KEY = process.env.ACCOUNT_B_PRIVATE_KEY || (
	'1111111111111111111111111111111111111111111111111111111111111111');
const accountBKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_B_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const accountAAddress = facade.network.publicKeyToAddress(
	accountAKeyPair.publicKey);
const accountBAddress = facade.network.publicKeyToAddress(
	accountBKeyPair.publicKey);
console.log('Account A:', accountAAddress.toString());
console.log('Account B:', accountBAddress.toString()); // [<step-1]

try {
	// Fetch recommended fees [>step-2]
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier); // [<step-2]

	// Embedded tx 1: Account A transfers 10 XYM to Account B [>step-3]
	const embeddedTransaction1 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				accountBAddress,
				[
					new descriptors.UnresolvedMosaicDescriptor(
						generateMosaicAliasId('symbol.xym'),
						new models.Amount(10_000_000n)) // 10 XYM
				],
				undefined),
			accountAKeyPair.publicKey);

	// Embedded tx 2: Account B transfers 1 custom mosaic to Account A
	const customMosaicId = 0x6D1314BE751B62C2n;
	const embeddedTransaction2 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				accountAAddress,
				[
					new descriptors.UnresolvedMosaicDescriptor(
						customMosaicId,
						new models.Amount(1n)) // 1 custom mosaic
				],
				undefined),
			accountBKeyPair.publicKey); // [<step-3]

	// Build the aggregate transaction [>step-4]
	const embeddedTransactions = [
		embeddedTransaction1, embeddedTransaction2];
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(embeddedTransactions),
			embeddedTransactions,
			undefined),
		accountAKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60,
		1);
	console.log('Built aggregate transaction without signatures:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));
	// [<step-4]
	// --- ACCOUNT A (Initiator) --- [>step-5]
	console.log('[Account A] Signing the aggregate...');
	const signatureA = facade.signTransaction(
		accountAKeyPair, transaction);
	const transactionPayload = facade.transactionFactory.static
		.attachSignature(transaction, signatureA);
	const payloadFormatted = JSON.stringify(
		JSON.parse(transactionPayload), null, 2);
	console.log('[Account A] Payload ready to share:\n',
		payloadFormatted);

	// --- OFF-CHAIN COORDINATION ---
	// Account A sends the payload to Account B
	const sharedPayload = transactionPayload;
	console.log('[Account A] ==> Payload sent to Account B (offchain)');
	// [<step-5]
	// --- ACCOUNT B (Cosignatory) --- [>step-6]
	const payloadHex = JSON.parse(sharedPayload).payload;
	const receivedTransaction = facade.transactionFactory.static
		.deserialize(Buffer.from(payloadHex, 'hex'));

	console.log('[Account B] Cosigning...');
	const cosignatureB = facade.cosignTransaction(
		accountBKeyPair, receivedTransaction);
	const cosignatureFormatted = JSON.stringify(
		cosignatureB.toJson(), null, 2);
	console.log('[Account B] Cosignature created:',
		cosignatureFormatted);

	// --- OFF-CHAIN COORDINATION ---
	// Account B sends the cosignature back to Account A
	const sharedCosignature = cosignatureB;
	console.log('[Account B] <== Cosignature sent back to Account A',
		'(offchain)'); // [<step-6]

	// --- ACCOUNT A (Initiator) --- [>step-7]
	// Add cosignature to the transaction and rebuild payload
	transaction.cosignatures.push(sharedCosignature);
	const transactionPayloadFinal = facade.transactionFactory.static
		.toJson(transaction);
	const jsonPayload = transactionPayloadFinal;
	console.log('[Account A] Ready to announce'); // [<step-7]

	// Announce the transaction [>step-8]
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);
	await announceTransaction(jsonPayload, 'transaction'); // [<step-8]

	// Wait for confirmation [>step-9]
	await waitForConfirmation(transactionHash, 'transaction');
	// [<step-9]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
