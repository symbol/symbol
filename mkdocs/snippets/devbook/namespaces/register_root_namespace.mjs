import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	SymbolFacade,
	descriptors,
	generateNamespaceId,
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
	// Build the namespace name [>step-3]
	const namespaceName =
		process.env.ROOT_NAMESPACE || `ns_${Date.now()}`;
	console.log('Creating root namespace:', namespaceName);
	// [<step-3]
	// Build the transaction [>step-4]
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.NamespaceRegistrationTransactionV1Descriptor(
			new models.NamespaceId(0n),
			models.NamespaceRegistrationType.ROOT,
			new models.BlockDuration(86400n), // approximately 30 days
			undefined,
			namespaceName),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const signature = facade.signTransaction(
		signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	// Announce transaction
	await announceTransaction(jsonPayload, 'namespace registration');
	// [<step-5]
	// Wait for confirmation [>step-6]
	await waitForConfirmation(transactionHash, 'namespace registration');
	// [<step-6]
	// Retrieve the namespace [>step-7]
	const namespaceId = generateNamespaceId(namespaceName);
	const namespaceIdHex = namespaceId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log(
		'Namespace ID:',
		`${namespaceId} (0x${namespaceIdHex})`);

	const namespacePath = `/namespaces/${namespaceIdHex}`;
	console.log('Fetching namespace information from', namespacePath);
	const namespaceResponse = await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Namespace information:');
	console.log('  Registration type:', namespaceInfo.registrationType);
	const ownerAddress = Address.fromDecodedAddressHexString(
		namespaceInfo.ownerAddress);
	console.log('  Owner address:', ownerAddress.toString());
	console.log('  Start height:', namespaceInfo.startHeight);
	console.log('  End height:', namespaceInfo.endHeight); // [<step-7]
} catch (e) {
	console.error(e.message);
}
