import { PrivateKey } from 'symbol-sdk';
import {
	Address,
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

// Target address to link the namespace to
const targetAddress = new SymbolFacade.Address(
	process.env.TARGET_ADDRESS ||
	'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI');
console.log('Target address:', targetAddress.toString());
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
		new descriptors.AddressAliasTransactionV1Descriptor(
			new models.NamespaceId(namespaceId),
			targetAddress,
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
	await announceTransaction(jsonPayload, 'address alias transaction');
	await waitForConfirmation(
		transactionHash, 'address alias transaction');
	// [<step-5]
	// Retrieve the namespace to verify the alias [>step-6]
	const namespacePath = `/namespaces/${namespaceIdHex}`;
	console.log('Fetching namespace information from', namespacePath);
	const namespaceResponse = await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Alias information:');
	console.log('  Alias type:', namespaceInfo.alias.type);
	if (2 === namespaceInfo.alias.type) { // ADDRESS type
		const aliasedAddress = Address.fromDecodedAddressHexString(
			namespaceInfo.alias.address);
		console.log('  Linked address:', aliasedAddress.toString());
	}
	// [<step-6]
	// Send a transfer using the alias instead of a raw address [>step-7]
	console.log('Using alias in transfer:', namespaceName);

	// Encode the namespace ID as a recipient address
	const recipientPath = generateNamespacePath(namespaceName);
	const recipientId = recipientPath[recipientPath.length - 1];
	const recipientAddress = Address.fromNamespaceId(
		new models.NamespaceId(recipientId), facade.network.identifier);
	console.log('  Recipient address (alias):',
		recipientAddress.toString());

	const test_transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.TransferTransactionV1Descriptor(
			recipientAddress,
			[
				new descriptors.UnresolvedMosaicDescriptor(
					generateMosaicAliasId('symbol.xym'),
					new models.Amount(1_000_000n)) // 1 XYM
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
