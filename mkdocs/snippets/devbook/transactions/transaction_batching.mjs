import { PrivateKey } from 'symbol-sdk';
import {
	Address,
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
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer public key:', signerKeyPair.publicKey.toString());
console.log('Signer address:', signerAddress.toString());

const RECIPIENT_1 = process.env.RECIPIENT_1 ||
	'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI';
const RECIPIENT_2 = process.env.RECIPIENT_2 ||
	'TCD4NC5VIE2EEB3BCV5JRLBNJXYDW5Q5JK547MI';
const recipient1Hex = Buffer.from(
	new Address(RECIPIENT_1).bytes).toString('hex').toUpperCase();
const recipient2Hex = Buffer.from(
	new Address(RECIPIENT_2).bytes).toString('hex').toUpperCase();
console.log(`Recipient 1: ${RECIPIENT_1} (${recipient1Hex})`);
console.log(`Recipient 2: ${RECIPIENT_2} (${recipient2Hex})`);
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
	// Embedded tx 1: Send 5 XYM to Recipient 1 [>step-3]
	const xymMosaicId = generateMosaicAliasId('symbol.xym');
	const embeddedTx1 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				new Address(RECIPIENT_1),
				[
					new descriptors.UnresolvedMosaicDescriptor(
						xymMosaicId,
						new models.Amount(5_000_000n)) // 5 XYM
				],
				undefined),
			signerKeyPair.publicKey);

	// Embedded tx 2: Send 3 XYM to Recipient 2
	const embeddedTx2 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				new Address(RECIPIENT_2),
				[
					new descriptors.UnresolvedMosaicDescriptor(
						xymMosaicId,
						new models.Amount(3_000_000n)) // 3 XYM
				],
				undefined),
			signerKeyPair.publicKey);
	// [<step-3]
	// Build the aggregate transaction [>step-4]
	const embeddedTransactions = [embeddedTx1, embeddedTx2];
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(embeddedTransactions),
			embeddedTransactions,
			undefined),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	console.log('Built aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const signature = facade.signTransaction(
		signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static
		.attachSignature(transaction, signature);

	// Announce the transaction
	await announceTransaction(jsonPayload, 'transaction');
	// [<step-5]
	// Wait for confirmation [>step-6]
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);
	await waitForConfirmation(transactionHash, 'transaction');
	// [<step-6]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
