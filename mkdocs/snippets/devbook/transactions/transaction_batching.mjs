import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	generateMosaicAliasId,
	models,
	NetworkTimestamp,
	SymbolFacade
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);
// [>step-1]
const SIGNER_PRIVATE_KEY =
	process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const signerAddress = facade.network.publicKeyToAddress(
	signerKeyPair.publicKey);
console.log('Signer public key:', signerKeyPair.publicKey.toString());
console.log('Signer address:', signerAddress.toString());

const RECIPIENT_1 = process.env.RECIPIENT_1 ||
	'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI';
const RECIPIENT_2 = process.env.RECIPIENT_2 ||
	'TCD4NC5VIE2EEB3BCV5JRLBNJXYDW5Q5JK547MI';
const recipient1Hex = Buffer.from(
	new Address(RECIPIENT_1).bytes).toString('hex').toUpperCase();
const recipient2Hex = Buffer.from(
	new Address(RECIPIENT_2).bytes).toString('hex').toUpperCase();
console.log(`Recipient 1: ${RECIPIENT_1} (${recipient1Hex})`);
console.log(`Recipient 2: ${RECIPIENT_2} (${recipient2Hex})`);
// [<step-1]
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
	// Embedded tx 1: Send 5 XYM to Recipient 1 [>step-3]
	const xymMosaicId = generateMosaicAliasId('symbol.xym');
	const embeddedTx1 = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		recipientAddress: RECIPIENT_1,
		mosaics: [{
			mosaicId: xymMosaicId,
			amount: 5_000_000n  // 5 XYM
		}]
	});

	// Embedded tx 2: Send 3 XYM to Recipient 2
	const embeddedTx2 = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		recipientAddress: RECIPIENT_2,
		mosaics: [{
			mosaicId: xymMosaicId,
			amount: 3_000_000n  // 3 XYM
		}]
	});
	// [<step-3]
	// Build the aggregate transaction [>step-4]
	const embeddedTransactions = [embeddedTx1, embeddedTx2];
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			embeddedTransactions),
		transactions: embeddedTransactions
	});
	transaction.fee = new models.Amount(
		feeMult * transaction.size);
	console.log('Built aggregate transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));
	// [<step-4]
	// Sign transaction and generate final payload [>step-5]
	const signature = facade.signTransaction(
		signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static
		.attachSignature(transaction, signature);

	// Announce the transaction
	const announcePath = '/transactions';
	console.log('Announcing transaction to', announcePath);
	const announceResponse = await fetch(
		`${NODE_URL}${announcePath}`, {
			method: 'PUT',
			headers: { 'Content-Type': 'application/json' },
			body: jsonPayload
		});
	console.log('  Response:', await announceResponse.text());
	// [<step-5]
	// Wait for confirmation [>step-6]
	const transactionHash =
		facade.hashTransaction(transaction).toString();
	const statusPath = `/transactionStatus/${transactionHash}`;
	console.log('Waiting for confirmation from', statusPath);

	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const statusResponse = await fetch(
				`${NODE_URL}${statusPath}`);
			const status = await statusResponse.json();
			console.log('  Transaction status:', status.group);
			if (status.group === 'confirmed') {
				console.log('Transaction confirmed in', attempt,
					'seconds');
				break;
			}
			if (status.group === 'failed') {
				console.log('Transaction failed:', status.code);
				break;
			}
		} catch (e) {
			console.log('  Transaction status: unknown | Cause:',
				e.message);
		}
	} // [<step-6]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
