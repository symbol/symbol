import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateNamespacePath,
	generateMosaicAliasId
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
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);

	// Build the alias transaction
	const transaction = facade.transactionFactory.create({
		type: 'mosaic_alias_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		namespaceId: namespaceId,
		mosaicId: mosaicId,
		aliasAction: 'link'
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);

	// Sign transaction and generate final payload
	const signature = facade.signTransaction(
		signerKeyPair, transaction);
	const jsonPayload =
		facade.transactionFactory.static.attachSignature(
			transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	const transactionHash =
		facade.hashTransaction(transaction).toString();
	console.log('Transaction hash:', transactionHash);

	// Announce transaction
	console.log('Announcing mosaic alias transaction to /transactions');
	const announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());

	// Wait for confirmation
	console.log('Waiting for transaction confirmation...');
	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));

		try {
			const statusUrl =
				`${NODE_URL}/transactionStatus/${transactionHash}`;
			const statusResponse = await fetch(statusUrl);

			if (!statusResponse.ok) {
				console.log('  Transaction status: unknown');
				continue;
			}

			const status = await statusResponse.json();
			console.log('  Transaction status:', status.group);

			if (status.group === 'confirmed') {
				console.log('Mosaic alias transaction confirmed in',
					attempt, 'seconds');
				break;
			}

			if (status.group === 'failed') {
				throw new Error(
					`Mosaic alias transaction failed: ${status.code}`);
			}
		} catch (error) {
			if (error.message.includes('failed:')) {
				throw error;
			}
			console.log('  Transaction status: unknown');
		}
	}

	// Retrieve the namespace to verify the alias
	const namespacePath =
		`/namespaces/${namespaceId.toString(16)}`;
	console.log(
		'Fetching namespace information from', namespacePath);
	const namespaceResponse =
		await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Alias information:');
	console.log('  Alias type:', namespaceInfo.alias.type);
	if (namespaceInfo.alias.type === 1) { // MOSAIC type
		console.log('  Linked mosaic ID:', namespaceInfo.alias.mosaicId);
	}

	// Send a transfer using the alias instead of a raw mosaic ID
	console.log('Using alias in transfer:', namespaceName);

	// Convert namespace to mosaic alias ID
	const mosaicAliasId = generateMosaicAliasId(namespaceName);

	const transferTx = facade.transactionFactory.create({
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
		`${mosaicAliasId} (0x${mosaicAliasId.toString(16)})`);
} catch (e) {
	console.error(e.message);
}
