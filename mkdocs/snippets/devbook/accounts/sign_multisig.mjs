import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
	generateMosaicAliasId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);
// [>step-1]
const MULTISIG_PRIVATE_KEY = process.env.MULTISIG_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000001');
const multisigKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(MULTISIG_PRIVATE_KEY));
console.log(`Multisig public key: ${multisigKeyPair.publicKey}`);
const COSIGNATORY0_PRIVATE_KEY = process.env.COSIGNATORY0_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000002');
const cosignatoryKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(COSIGNATORY0_PRIVATE_KEY));
console.log(`Cosignatory public key: ${cosignatoryKeyPair.publicKey}`);
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

	// Fetch recommended fees
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);
	// [<step-2]
	// Build the embedded transfer transaction [>step-3]
	const transferTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: multisigKeyPair.publicKey.toString(),
		recipientAddress: facade.network.publicKeyToAddress(
			multisigKeyPair.publicKey).toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 1_000_000n // 1 XYM
		}]
	});
	// [<step-3]
	// Build the wrapper aggregate transaction [>step-4]
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		// This is the account that will pay for the transaction
		signerPublicKey: cosignatoryKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			[transferTransaction]),
		transactions: [transferTransaction]
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);
	// [<step-4]
	// Sign the aggregate transaction using the cosignatory's signature [>step-5]
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(cosignatoryKeyPair, transaction));
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

	// Wait for confirmation
	const transactionHash = facade.hashTransaction(transaction).toString();
	const statusPath = `/transactionStatus/${transactionHash}`;
	console.log('Waiting for confirmation from', statusPath);

	for (let attempt = 1; 60 >= attempt; ++attempt) {
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
		await new Promise(resolve => { setTimeout(resolve, 1000); });
		if (60 === attempt)
			console.warn('Confirmation took too long.');
	} // [<step-6]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
