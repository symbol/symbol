import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	descriptors,
	generateMosaicAliasId
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://001-sai-dual.symboltest.net:3001';
console.log('Using node', NODE_URL);

const SIGNER_PRIVATE_KEY =
	'EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');

try {
	// Fetch current network time
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
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
	const typedDescriptor =
		new descriptors.TransferTransactionV1Descriptor(
			facade.network.publicKeyToAddress(signerKeyPair.publicKey),
			[
				new descriptors.UnresolvedMosaicDescriptor(
					new models.UnresolvedMosaicId(
						generateMosaicAliasId('symbol.xym')),
					new models.Amount(1_000_000n) // 1 XYM
				)
			]
		);
	const transaction = facade.createTransactionFromTypedDescriptor(
		typedDescriptor, signerKeyPair.publicKey, 0, 2 * 60 * 60);
	transaction.fee = new models.Amount(feeMult * transaction.size);

	// Sign transaction and generate final payload
	const signature = facade.signTransaction(signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });

	// Announce the transaction
	const announcePath = '/transactions';
	console.log('Announcing transaction to', announcePath);
	const announceResponse = await fetch(`${NODE_URL}${announcePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());

	// Wait for confirmation
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	const statusPath = `/transactionStatus/${transactionHash}`;
	console.log('Waiting for confirmation from', statusPath);

	let attempt = 0;

	function pollStatus() {
		attempt++;

		if (attempt > 60) {
			console.warn('Confirmation took too long.');
			return;
		}

		return fetch(`${NODE_URL}${statusPath}`)
			.then(response => {
				if (!response.ok) {
					console.log('  Transaction status: unknown | Cause:',
						response.statusText);
					// HTTP error: schedule a retry
					return new Promise(resolve =>
						setTimeout(resolve, 1000)).then(pollStatus);
				}
				return response.json();
			})
			.then(status => {
				// Skip if previous step scheduled a retry
				if (!status) return;

				console.log('  Transaction status:', status.group);

				if (status.group === 'confirmed') {
					console.log('Transaction confirmed in', attempt,
						'seconds');
				} else if (status.group === 'failed') {
					console.log('Transaction failed:', status.code);
				} else {
					// Transaction unconfirmed: schedule a retry
					return new Promise(resolve =>
						setTimeout(resolve, 1000)).then(pollStatus);
				}
			});
	}
	pollStatus();
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
