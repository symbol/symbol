import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	NetworkTimestamp,
	SymbolFacade,
	generateNamespaceId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer address:', signerAddress.toString());

try {
	// Fetch current network time
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

	// Build the transaction [>step-1]
	const rootNamespaceName = 'ns_root';
	const childNamespaceName = `sub_${Date.now()}`;
	const fullNamespaceName = `${rootNamespaceName}.${childNamespaceName}`;
	console.log('Creating child namespace:', fullNamespaceName);

	// Generate the parent namespace ID from the root name
	const parentId = generateNamespaceId(rootNamespaceName);
	console.log(
		'Parent namespace ID:', `0x${parentId.toString(16)}`);

	const transaction = facade.transactionFactory.create({
		type: 'namespace_registration_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		registrationType: 'child',
		parentId,
		name: childNamespaceName
	});
	transaction.fee = new models.Amount(feeMultiplier * transaction.size);
	// [<step-1]
	// Sign transaction and generate final payload
	const signature = facade.signTransaction(
		signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	const transactionHash = facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	// Announce transaction
	console.log('Announcing namespace registration to /transactions');
	const announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());

	// Wait for confirmation
	console.log('Waiting for namespace registration confirmation...');
	const statusPath = `/transactionStatus/${transactionHash}`;
	for (let attempt = 1; 60 >= attempt; ++attempt) {
		await new Promise(resolve => { setTimeout(resolve, 1000); });
		const response = await fetch(`${NODE_URL}${statusPath}`);

		if (response.ok) {
			const status = await response.json();
			console.log('  Transaction status:', status.group);
			if ('confirmed' === status.group) {
				console.log('Transaction confirmed in', attempt,
					'seconds');
				break;
			}
			if ('failed' === status.group) {
				console.log('Transaction failed:', status.code);
				break;
			}
		} else {
			console.log('  Transaction status: unknown | Cause:',
				response.status
			);
		}
		if (60 === attempt)
			console.warn('Confirmation took too long.');
	}

	// Retrieve the namespace [>step-2]
	const namespaceId = generateNamespaceId(
		childNamespaceName, parentId);
	console.log(
		'Child namespace ID:',
		`${namespaceId} (0x${namespaceId.toString(16)})`);

	const namespacePath = `/namespaces/${namespaceId.toString(16)}`;
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
	console.log('  End height:', namespaceInfo.endHeight); // [<step-2]
} catch (e) {
	console.error(e.message);
}
