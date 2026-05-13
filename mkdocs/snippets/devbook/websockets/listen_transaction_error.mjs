import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
	generateMosaicAliasId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
const WS_URL = `${NODE_URL.replace('http', 'ws')}/ws`;
console.log(`Using node ${NODE_URL}`);
// [>step-1]
const MONITOR_ADDRESS = process.env.MONITOR_ADDRESS ||
	'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I';
console.log(`Monitoring address: ${MONITOR_ADDRESS}`);

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const facade = new SymbolFacade('testnet');
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));
// [<step-1]
try {
	// Connect to WebSocket [>step-2]
	const websocket = new WebSocket(WS_URL);
	const uid = await new Promise(resolve => {
		websocket.addEventListener('message', event => {
			const message = JSON.parse(event.data);
			resolve(message.uid);
		}, { once: true });
	});
	console.log(`Connected to ${WS_URL} with uid ${uid}`);
	// [<step-2]
	// Subscribe to status channel [>step-3]
	const channel = `status/${MONITOR_ADDRESS}`;
	websocket.send(JSON.stringify({ uid, subscribe: channel }));
	console.log('Subscribed to status channel');
	// [<step-3]
	// Build a transfer transaction with a non-existent mosaic [>step-4]
	const timeResponse = await fetch(`${NODE_URL}/node/time`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);

	const feeResponse = await fetch(
		`${NODE_URL}/network/fees/transaction`);
	const feeJSON = await feeResponse.json();
	const feeMultiplier = Math.max(
		feeJSON.medianFeeMultiplier, feeJSON.minFeeMultiplier);

	const transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: MONITOR_ADDRESS,
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.unknown'),
			amount: 1n
		}]
	});
	transaction.fee = new models.Amount(feeMultiplier * transaction.size);

	const signature = facade.signTransaction(signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	const transactionHash = facade.hashTransaction(transaction).toString(); // [<step-4]
	// [>step-5]
	const rejected = new Promise(resolve => {
		websocket.addEventListener('message', event => {
			const msg = JSON.parse(event.data);
			const txHash = msg.data.hash;
			const code = msg.data.code;
			console.log(
				`Transaction ${txHash.substring(0, 16)}... ` +
				`rejected with code: ${code}`);

			if (txHash === transactionHash)
				resolve();
		});
	});
	await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log(
		'Announced transaction ' +
		`${transactionHash.substring(0, 16)}...`);

	// Wait for error via WebSocket
	await rejected;
	// [<step-5]
	// Unsubscribe before closing [>step-6]
	websocket.send(JSON.stringify({ uid, unsubscribe: channel }));
	console.log('Unsubscribed from status channel');
	websocket.close(); // [<step-6]
} catch (error) {
	console.error(error);
}
