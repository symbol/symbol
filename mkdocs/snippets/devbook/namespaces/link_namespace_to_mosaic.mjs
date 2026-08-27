import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	descriptors,
	generateMosaicAliasId,
	generateNamespacePath,
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
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString()); // [<step-1]
// [>step-2]
const namespaceName = process.env.NAMESPACE_NAME || 'my_namespace';
console.log('Namespace name:', namespaceName);

const nsPath = generateNamespacePath(namespaceName);
const namespaceId = nsPath[nsPath.length - 1];
const namespaceIdHex = namespaceId.toString(16)
	.toUpperCase().padStart(16, '0');
console.log(
	'Namespace ID:',
	`${namespaceId} (0x${namespaceIdHex})`);

// Target mosaic ID to link the namespace to
const MOSAIC_ID = process.env.MOSAIC_ID || '45C8C3733983AAC2';
const mosaicId = BigInt(`0x${MOSAIC_ID}`);
const mosaicIdHex = mosaicId.toString(16)
	.toUpperCase().padStart(16, '0');
console.log('Mosaic ID:', `${mosaicId} (0x${mosaicIdHex})`);
// [<step-2]
try {
	// Fetch recommended fees [>step-3]
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier);
	// [<step-3]
	// Build the alias transaction [>step-4]
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.MosaicAliasTransactionV1Descriptor(
			new models.NamespaceId(namespaceId),
			new models.MosaicId(mosaicId),
			models.AliasAction.LINK),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	// Announce and confirm transaction
	await announceTransaction(jsonPayload, 'mosaic alias transaction');
	await waitForConfirmation(
		transactionHash, 'mosaic alias transaction');
	// [<step-5]
	// Retrieve the namespace to verify the alias [>step-6]
	const namespacePath = `/namespaces/${namespaceIdHex}`;
	console.log('Fetching namespace information from', namespacePath);
	const namespaceResponse = await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Alias information:');
	console.log('  Alias type:', namespaceInfo.alias.type);
	if (1 === namespaceInfo.alias.type) { // MOSAIC type
		console.log('  Linked mosaic ID:', namespaceInfo.alias.mosaicId);
	}
	// [<step-6]
	// Send a transfer using the alias instead of a raw mosaic ID [>step-7]
	console.log('Using alias in transfer:', namespaceName);

	// Convert namespace to mosaic alias ID
	const mosaicAliasId = generateMosaicAliasId(namespaceName);
	const mosaicAliasIdHex = mosaicAliasId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log('  Mosaic ID (alias):',
		`${mosaicAliasId} (0x${mosaicAliasIdHex})`);

	const test_transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.TransferTransactionV1Descriptor(
			facade.network.publicKeyToAddress(signerKeyPair.publicKey),
			[
				new descriptors.UnresolvedMosaicDescriptor(
					generateMosaicAliasId(namespaceName),
					new models.Amount(1n))
			],
			undefined),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	const testJsonPayload =
		facade.transactionFactory.static.attachSignature(
			test_transaction,
			facade.signTransaction(signerKeyPair, test_transaction));
	console.log('Test transaction:');
	console.dir(test_transaction.toJson(), { colors: true });
	const testTransactionHash =
		facade.hashTransaction(test_transaction).toString();
	console.log('Transaction hash:', testTransactionHash);
	await announceTransaction(testJsonPayload, 'test transaction');
	await waitForConfirmation(testTransactionHash, 'test transaction');
	// [<step-7]
} catch (e) {
	console.error(e.message);
}
