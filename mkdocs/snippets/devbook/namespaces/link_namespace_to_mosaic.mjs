import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
	generateMosaicAliasId,
	generateNamespacePath,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);
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

// Target mosaic ID to link the namespace to
const mosaicId = BigInt(process.env.MOSAIC_ID || '0x45C8C3733983AAC2');
console.log('Mosaic ID:', `${mosaicId} (0x${mosaicId.toString(16)})`);
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
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);
	// [<step-3]
	// Build the alias transaction [>step-4]
	const transaction = facade.transactionFactory.create({
		type: 'mosaic_alias_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		namespaceId,
		mosaicId,
		aliasAction: 'link'
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const signature = facade.signTransaction(
		signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	const transactionHash = facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	// Announce transaction
	console.log('Announcing mosaic alias transaction to /transactions');
	const announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());
	// [<step-5]
	// Wait for confirmation [>step-6]
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
	// [<step-6]
	// Retrieve the namespace to verify the alias [>step-7]
	const namespacePath = `/namespaces/${namespaceId.toString(16)}`;
	console.log('Fetching namespace information from', namespacePath);
	const namespaceResponse = await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Alias information:');
	console.log('  Alias type:', namespaceInfo.alias.type);
	if (1 === namespaceInfo.alias.type) { // MOSAIC type
		console.log('  Linked mosaic ID:', namespaceInfo.alias.mosaicId);
	}
	// [<step-7]
	// Send a transfer using the alias instead of a raw mosaic ID [>step-8]
	console.log('Using alias in transfer:', namespaceName);

	// Convert namespace to mosaic alias ID
	const mosaicAliasId = generateMosaicAliasId(namespaceName);

	facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: facade.network.publicKeyToAddress(
			signerKeyPair.publicKey).toString(),
		mosaics: [{
			mosaicId: mosaicAliasId,
			amount: 1n
		}]
	});
	console.log('Transfer transaction:');
	console.log('  Mosaic ID (alias):',
		`${mosaicAliasId} (0x${mosaicAliasId.toString(16)})`); // [<step-8]
} catch (e) {
	console.error(e.message);
}
