import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	Address,
	models,
	generateNamespacePath,
	generateMosaicAliasId
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
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);
	// [<step-3]
	// Build the alias transaction [>step-4]
	const transaction = facade.transactionFactory.create({
		type: 'address_alias_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		namespaceId: namespaceId,
		address: targetAddress,
		aliasAction: 'link'
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
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
	console.log('Announcing address alias transaction to /transactions');
	const announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());
	// [<step-5]
	// Wait for confirmation [>step-6]
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
				console.log('Address alias transaction confirmed in',
					attempt, 'seconds');
				break;
			}

			if (status.group === 'failed') {
				throw new Error(
					`Address alias transaction failed: ${status.code}`);
			}
		} catch (error) {
			if (error.message.includes('failed:')) {
				throw error;
			}
			console.log('  Transaction status: unknown');
		}
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
	if (namespaceInfo.alias.type === 2) { // ADDRESS type
		const aliasedAddress = Address.fromDecodedAddressHexString(
			namespaceInfo.alias.address);
		console.log('  Linked address:', aliasedAddress.toString());
	}
	// [<step-7]
	// Send a transfer using the alias instead of a raw address [>step-8]
	console.log('Using alias in transfer:', namespaceName);

	// Encode the namespace ID as a recipient address
	const recipientPath = generateNamespacePath(namespaceName);
	const recipientId = recipientPath[recipientPath.length - 1];
	const recipientAddress = Address.fromNamespaceId(
		new models.NamespaceId(recipientId), facade.network.identifier);

	const transferTx = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: recipientAddress,
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n // 1 XYM
		}]
	});
	console.log('Transfer transaction:');
	console.log('  Recipient address (alias):',
		recipientAddress.toString()); // [<step-8]
} catch (e) {
	console.error(e.message);
}
