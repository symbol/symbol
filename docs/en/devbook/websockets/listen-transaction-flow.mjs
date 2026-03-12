import { PrivateKey } from 'symbol-sdk';
import { SymbolFacade, NetworkTimestamp, models } from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
const WS_URL = NODE_URL.replace('http', 'ws') + '/ws';
console.log(`Using node ${NODE_URL}`);

const MONITOR_ADDRESS = process.env.MONITOR_ADDRESS
	|| 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I';
console.log(`Monitoring address: ${MONITOR_ADDRESS}`);

const SIGNER_PRIVATE_KEY = process.env.SIGNER_PRIVATE_KEY
	|| '0000000000000000000000000000000000000000000000000000000000000000';
const facade = new SymbolFacade('testnet');
const signerKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(SIGNER_PRIVATE_KEY));

try {
	// Connect to WebSocket
	const websocket = new WebSocket(WS_URL);
	const uid = await new Promise((resolve) => {
		websocket.addEventListener('message', (event) => {
			const message = JSON.parse(event.data);
			resolve(message.uid);
		}, { once: true });
	});
	console.log(`Connected to ${WS_URL} with uid ${uid}`);

	// Subscribe to transaction channels
	const channels = [
		`unconfirmedAdded/${MONITOR_ADDRESS}`,
		`unconfirmedRemoved/${MONITOR_ADDRESS}`,
		`confirmedAdded/${MONITOR_ADDRESS}`,
	];
	for (const channel of channels) {
		websocket.send(JSON.stringify({ uid, subscribe: channel }));
		const name = channel.split('/')[0];
		console.log(`Subscribed to ${name} channel`);
	}

	// Build and announce a transfer transaction
	const timeResponse = await fetch(`${NODE_URL}/node/time`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);

	const feeResponse = await fetch(
		`${NODE_URL}/network/fees/transaction`);
	const feeJSON = await feeResponse.json();
	const feeMult = Math.max(
		feeJSON.medianFeeMultiplier, feeJSON.minFeeMultiplier);

	const transaction = facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
		signerPublicKey: signerKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		recipientAddress: MONITOR_ADDRESS,
	});
	transaction.fee = new models.Amount(feeMult * transaction.size);

	const signature = facade.signTransaction(signerKeyPair, transaction);
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction, signature);
	const transactionHash = facade.hashTransaction(transaction).toString();

	const confirmed = new Promise((resolve) => {
		websocket.addEventListener('message', (event) => {
			const message = JSON.parse(event.data);
			const topic = message.topic;
			const messageHash = message.data.meta.hash;
			const name = topic.split('/')[0];
			console.log(
				`${name}: hash=${messageHash.substring(0, 16)}...`);

			if (name === 'confirmedAdded'
				&& messageHash === transactionHash) {
					console.log(
						`Transaction ${transactionHash.substring(0, 16)}`
						+ '... confirmed');
					resolve();
			}
		});
	});
	await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log(
		'Announced transaction '
		+ `${transactionHash.substring(0, 16)}...`);

	// Wait for confirmation via WebSocket
	await confirmed;

	// Unsubscribe before closing
	for (const channel of channels) {
		websocket.send(JSON.stringify({ uid, unsubscribe: channel }));
	}
	console.log('Unsubscribed from all channels');
	websocket.close();
} catch (error) {
	console.error(error);
}
