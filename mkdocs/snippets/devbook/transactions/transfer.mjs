import { PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateMosaicAliasId
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);
// [>step-1]
const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));
// [<step-1]
const facade = new SymbolFacade('testnet');

try {
	// Fetch current network time [>step-2]
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log('  Network time:', timestamp.timestamp,
		'ms since nemesis');
	// [<step-2]
	// Fetch recommended fees [>step-3]
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);
	// [<step-3]
	// Build the transaction [>step-4]
	const transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: facade.network.publicKeyToAddress(
			signerKeyPair.publicKey).toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n // 1 XYM
		}]
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const signature = facade.signTransaction(signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	console.log('Built transaction:');
	console.dir(transaction.toJson(), { colors: true });
	// [<step-5]
	// Announce the transaction [>step-6]
	const announcePath = '/transactions';
	console.log('Announcing transaction to', announcePath);
	const announceResponse = await fetch(`${NODE_URL}${announcePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log('  Response:', await announceResponse.text());
	// [<step-6]
	// Wait for confirmation [>step-7]
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
	pollStatus(); // [<step-7]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
