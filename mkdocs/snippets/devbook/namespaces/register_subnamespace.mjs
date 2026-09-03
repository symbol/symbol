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

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

try {
	// Fetch recommended fees
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier);

	// Build the subnamespace name [>step-1]
	const rootNamespaceName = process.env.ROOT_NAMESPACE || 'ns_root';
	const subnamespaceName =
		process.env.SUBNAMESPACE || `sub_${Date.now()}`;
	const fullNamespaceName =
		`${rootNamespaceName}.${subnamespaceName}`;
	console.log('Creating subnamespace:', fullNamespaceName);

	// Generate the parent namespace ID from the root name
	const parentId = generateNamespaceId(rootNamespaceName);
	const parentIdHex = parentId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log('Parent namespace ID:', `0x${parentIdHex}`);
	// [<step-1]
	// Build the transaction [>step-2]
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.NamespaceRegistrationTransactionV1Descriptor(
			new models.NamespaceId(0n),
			models.NamespaceRegistrationType.CHILD,
			undefined,
			new models.NamespaceId(parentId),
			subnamespaceName),
		signerKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);
	// [<step-2]
	// Sign transaction and generate final payload
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

	// Wait for confirmation
	await waitForConfirmation(transactionHash, 'namespace registration');

	// Retrieve the namespace [>step-3]
	const namespaceId = generateNamespaceId(
		subnamespaceName, parentId);
	const namespaceIdHex = namespaceId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log(
		'Child namespace ID:',
		`${namespaceId} (0x${namespaceIdHex})`);

	const namespacePath = `/namespaces/${namespaceIdHex}`;
	console.log(
		'Fetching namespace information from', namespacePath);
	const namespaceResponse = await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Namespace information:');
	console.log(
		'  Registration type:', namespaceInfo.registrationType);
	const ownerAddress = Address.fromDecodedAddressHexString(
		namespaceInfo.ownerAddress);
	console.log('  Owner address:', ownerAddress.toString());
	console.log('  Parent ID:', namespaceInfo.parentId);
	console.log('  Depth:', namespaceInfo.depth);
	console.log('  Level 0:', namespaceInfo.level0);
	if (1 <= namespaceInfo.depth)
		console.log('  Level 1:', namespaceInfo.level1);
	if (2 <= namespaceInfo.depth && namespaceInfo.level2)
		console.log('  Level 2:', namespaceInfo.level2);
	console.log('  Start height:', namespaceInfo.startHeight);
	console.log('  End height:', namespaceInfo.endHeight); // [<step-3]
} catch (e) {
	console.error(e.message);
}
