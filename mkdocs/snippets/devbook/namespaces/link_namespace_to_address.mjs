import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	NetworkTimestamp,
	SymbolFacade,
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
console.log(
	'Namespace ID:',
	`${namespaceId} (0x${namespaceId.toString(16)})`);

// Target address to link the namespace to
const targetAddress = new SymbolFacade.Address(
	process.env.TARGET_ADDRESS ||
	'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI');
console.log('Target address:', targetAddress.toString());
// [<step-2]
try {
	// Fetch current network time [>step-3]
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const receiveTimestamp =
		timeJSON.communicationTimestamps.receiveTimestamp;
	const timestamp = new NetworkTimestamp(receiveTimestamp);
	console.log('  Network time:', timestamp.timestamp,
		'ms since nemesis');

	// Fetch recommended fees
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
	const transaction = facade.transactionFactory.create({
		type: 'address_alias_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		namespaceId,
		address: targetAddress,
		aliasAction: 'link'
	});
	transaction.fee = new models.Amount(feeMultiplier * transaction.size);
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(signerKeyPair, transaction));
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	const transactionHash = facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	// Announce and confirm transaction
	await announceTransaction(jsonPayload, 'address alias transaction');
	await waitForConfirmation(transactionHash, 'address alias transaction');
	// [<step-5]
	// Retrieve the namespace to verify the alias [>step-6]
	const namespacePath = `/namespaces/${namespaceId.toString(16)}`;
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

	const test_transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress,
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n // 1 XYM
		}]
	});
	test_transaction.fee = new models.Amount(
		feeMultiplier * test_transaction.size);
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
