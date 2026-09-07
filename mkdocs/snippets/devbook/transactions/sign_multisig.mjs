import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	descriptors,
	generateMosaicAliasId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://reference.symboltest.net:3001';
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
const MULTISIG_PRIVATE_KEY = process.env.MULTISIG_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000001');
const multisigKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(MULTISIG_PRIVATE_KEY));
console.log(`Multisig public key: ${multisigKeyPair.publicKey}`);
const COSIGNATORY0_PRIVATE_KEY = process.env.COSIGNATORY0_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000002';
const cosignatoryKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(COSIGNATORY0_PRIVATE_KEY));
console.log(`Cosignatory public key: ${cosignatoryKeyPair.publicKey}`);
// [<step-1]
const facade = new SymbolFacade('testnet');

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

	// Build the embedded transfer transaction [>step-3]
	const transferTransaction =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				facade.network.publicKeyToAddress(
					multisigKeyPair.publicKey),
				[
					new descriptors.UnresolvedMosaicDescriptor(
						generateMosaicAliasId('symbol.xym'),
						new models.Amount(1_000_000n)) // 1 XYM
				],
				undefined),
			multisigKeyPair.publicKey); // [<step-3]

	// Build the wrapper aggregate transaction [>step-4]
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions([transferTransaction]),
			[transferTransaction],
			undefined),
		cosignatoryKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60); // [<step-4]

	// Sign the aggregate transaction using the cosignatory's signature [>step-5]
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(cosignatoryKeyPair, transaction));
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true }); // [<step-5]

	// Announce the transaction [>step-6]
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);
	await announceTransaction(jsonPayload, 'transaction');

	// Wait for confirmation
	await waitForConfirmation(transactionHash, 'transaction'); // [<step-6]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
