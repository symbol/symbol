import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateNamespaceId
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
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);

	// Build the transaction
	const namespaceName = `ns_${Date.now()}`;
	console.log('Creating root namespace:', namespaceName);

	const transaction = facade.transactionFactory.create({
		type: 'namespace_registration_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		registrationType: 'root',
		duration: 86400n, // approximately 30 days
		name: namespaceName
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
	console.log('Announcing namespace registration to /transactions');
	const announceResponse = await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());

	// Wait for confirmation
	console.log('Waiting for namespace registration confirmation...');
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
				console.log('Namespace registration confirmed in',
					attempt, 'seconds');
				break;
			}

			if (status.group === 'failed') {
				throw new Error(
					`Namespace registration failed: ${status.code}`);
			}
		} catch (error) {
			if (error.message.includes('failed:')) {
				throw error;
			}
			console.log('  Transaction status: unknown');
		}
	}

	// Retrieve the namespace
	const namespaceId = generateNamespaceId(namespaceName);
	console.log('Namespace ID:', `0x${namespaceId.toString(16)}`);

	const namespacePath =
		`/namespaces/${namespaceId.toString(16)}`;
	console.log(
		'Fetching namespace information from', namespacePath);
	const namespaceResponse =
		await fetch(`${NODE_URL}${namespacePath}`);
	const namespaceJSON = await namespaceResponse.json();
	const namespaceInfo = namespaceJSON.namespace;
	console.log('Namespace information:');
	console.log(
		'  Registration type:', namespaceInfo.registrationType);
	console.log('  Owner address:', namespaceInfo.ownerAddress);
	console.log('  Start height:', namespaceInfo.startHeight);
	console.log('  End height:', namespaceInfo.endHeight);
} catch (e) {
	console.error(e.message);
}
