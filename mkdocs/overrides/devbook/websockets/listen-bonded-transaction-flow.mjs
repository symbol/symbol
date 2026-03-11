import { Hash256, PrivateKey } from 'symbol-sdk';
import {
	generateMosaicAliasId,
	models,
	NetworkTimestamp,
	SymbolFacade
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
const WS_URL = NODE_URL.replace('http', 'ws') + '/ws';
console.log(`Using node ${NODE_URL}`);

const ACCOUNT_A_PRIVATE_KEY =
	process.env.ACCOUNT_A_PRIVATE_KEY
	|| '0000000000000000000000000000000000000000000000000000000000000000';
const ACCOUNT_B_PRIVATE_KEY =
	process.env.ACCOUNT_B_PRIVATE_KEY
	|| '1111111111111111111111111111111111111111111111111111111111111111';

const facade = new SymbolFacade('testnet');
const accountAKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_A_PRIVATE_KEY));
const accountBKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_B_PRIVATE_KEY));
const accountAAddress = facade.network
	.publicKeyToAddress(accountAKeyPair.publicKey);
const accountBAddress = facade.network
	.publicKeyToAddress(accountBKeyPair.publicKey);
console.log('Account A:', accountAAddress.toString());
console.log('Account B:', accountBAddress.toString());

try {
	// Fetch current network time
	const timeResponse = await fetch(`${NODE_URL}/node/time`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);

	// Fetch recommended fee multiplier
	const feeResponse = await fetch(
		`${NODE_URL}/network/fees/transaction`);
	const feeJSON = await feeResponse.json();
	const feeMultiplier = Math.max(
		feeJSON.medianFeeMultiplier, feeJSON.minFeeMultiplier);

	// [Account A] Build embedded transactions for the swap
	const embeddedTx1 = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		recipientAddress: accountBAddress.toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 10_000_000n
		}]
	});

	const customMosaicId = 0x6D1314BE751B62C2n;
	const embeddedTx2 = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: accountBKeyPair.publicKey.toString(),
		recipientAddress: accountAAddress.toString(),
		mosaics: [{ mosaicId: customMosaicId, amount: 1n }]
	});

	// Build the bonded aggregate transaction
	const embeddedTxs = [embeddedTx1, embeddedTx2];
	const bondedTx = facade.transactionFactory.create({
		type: 'aggregate_bonded_transaction_v3',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash:
			facade.static.hashEmbeddedTransactions(embeddedTxs),
		transactions: embeddedTxs
	});
	bondedTx.fee = new models.Amount(
		feeMultiplier * (bondedTx.size + 104));

	// Sign the bonded aggregate
	const bondedSignature = facade.signTransaction(
		accountAKeyPair, bondedTx);
	const bondedPayload = facade.transactionFactory
		.static.attachSignature(bondedTx, bondedSignature);
	const bondedHash = facade
		.hashTransaction(bondedTx).toString();
	console.log('[Account A] Bonded aggregate hash: '
		+ `${bondedHash.substring(0, 16)}...`);

	// Create the hash lock transaction
	const hashLock = facade.transactionFactory.create({
		type: 'hash_lock_transaction_v1',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		mosaic: {
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 10_000_000n
		},
		duration: 100n,
		hash: bondedHash
	});
	hashLock.fee = new models.Amount(feeMultiplier * hashLock.size);
	const hashLockSignature = facade.signTransaction(
		accountAKeyPair, hashLock);
	const hashLockPayload = facade.transactionFactory
		.static.attachSignature(hashLock, hashLockSignature);
	const hashLockHash = facade
		.hashTransaction(hashLock).toString();

	// Confirm hash lock via WebSocket
	const lockWebSocket = new WebSocket(WS_URL);
	const lockUid = await new Promise((resolve) => {
		lockWebSocket.addEventListener('message', (event) => {
			const message = JSON.parse(event.data);
			resolve(message.uid);
		}, { once: true });
	});

	const addressA = accountAAddress.toString();
	const lockChannels = [
		`confirmedAdded/${addressA}`,
		`status/${addressA}`,
	];
	for (const channel of lockChannels) {
		lockWebSocket.send(JSON.stringify({
			uid: lockUid, subscribe: channel
		}));
	}

	// Announce hash lock
	await fetch(`${NODE_URL}/transactions`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: hashLockPayload
	});
	console.log('[Account A] Announced hash lock '
		+ `${hashLockHash.substring(0, 16)}...`);

	// Wait for hash lock confirmation
	await new Promise((resolve, reject) => {
		lockWebSocket.addEventListener('message', (event) => {
			const message = JSON.parse(event.data);
			const name = message.topic.split('/')[0];

			if (name === 'confirmedAdded'
				&& message.data.meta.hash === hashLockHash) {
				console.log('Hash lock confirmed');
				resolve();
			}

			if (name === 'status' && message.data.hash === hashLockHash) {
				reject(new Error(
					'Hash lock failed: ' + message.data.code));
			}
		});
	});

	for (const channel of lockChannels) {
		lockWebSocket.send(JSON.stringify({
			uid: lockUid, unsubscribe: channel
		}));
	}
	lockWebSocket.close();

	// [Account B] Connect to WebSocket for bonded flow
	const websocket = new WebSocket(WS_URL);
	const uid = await new Promise((resolve) => {
		websocket.addEventListener('message', (event) => {
			const message = JSON.parse(event.data);
			resolve(message.uid);
		}, { once: true });
	});
	console.log(`[Account B] Connected to ${WS_URL} with uid ${uid}`);

	// Subscribe to bonded transaction channels
	const addressB = accountBAddress.toString();
	const channels = [
		`partialAdded/${addressB}`,
		`partialRemoved/${addressB}`,
		`cosignature/${addressB}`,
		`unconfirmedAdded/${addressB}`,
		`unconfirmedRemoved/${addressB}`,
		`confirmedAdded/${addressB}`,
		`status/${addressB}`,
	];
	for (const channel of channels) {
		websocket.send(JSON.stringify({uid, subscribe: channel}));
		const name = channel.split('/')[0];
		console.log(`[Account B] Subscribed to ${name} channel`);
	}

	// [Account B] Listen for bonded transaction flow
	const confirmed = new Promise((resolve, reject) => {
		websocket.addEventListener('message', (event) => {
			const message = JSON.parse(event.data);
			const topic = message.topic;
			const name = topic.split('/')[0];

			if (name === 'cosignature') {
				const signer = message.data.signerPublicKey;
				console.log(
					`cosignature: signer=${signer.substring(0, 16)}...`);

			} else if (name === 'status') {
				const statusHash = message.data.hash;
				console.log(
					`status: hash=${statusHash.substring(0, 16)}...`);
				if (statusHash === bondedHash) {
					reject(new Error(
						'Transaction failed: ' + message.data.code));
					return;
				}

			} else if (name === 'partialAdded') {
				const messageHash = message.data.meta.hash;
				console.log('partialAdded: hash='
					+ `${messageHash.substring(0, 16)}...`);
				if (messageHash === bondedHash) {
					const cosignature =
						SymbolFacade.cosignTransactionHash(
							accountBKeyPair,
							new Hash256(bondedHash), true);
					const cosignaturePayload = JSON.stringify({
						version: cosignature.version.toString(),
						signerPublicKey:
							cosignature.signerPublicKey.toString(),
						signature: cosignature.signature.toString(),
						parentHash: cosignature.parentHash.toString()
					});
					fetch(`${NODE_URL}/transactions/cosignature`, {
						method: 'PUT',
						headers: {'Content-Type': 'application/json'},
						body: cosignaturePayload
					}).then(() => console.log(
						'[Account B] Submitted cosignature'))
					.catch((err) => console.error(
						'Cosignature failed:', err));
				}

			} else if (name === 'confirmedAdded') {
				const messageHash = message.data.meta.hash;
				console.log(`confirmedAdded: hash=`
					+ `${messageHash.substring(0, 16)}...`);
				if (messageHash === bondedHash) {
					console.log('Transaction '
						+ bondedHash.substring(0, 16) + '... confirmed');
					resolve();
				}

			} else {
				const messageHash = message.data.meta.hash;
				console.log(
					`${name}: hash=${messageHash.substring(0, 16)}...`);
			}
		});
	});

	// [Account A] Announce bonded aggregate
	await fetch(`${NODE_URL}/transactions/partial`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: bondedPayload
	});
	console.log('[Account A] Announced bonded '
		+ `${bondedHash.substring(0, 16)}...`);

	// Wait for confirmation via WebSocket
	await confirmed;

	// Unsubscribe before closing
	for (const channel of channels) {
		websocket.send(JSON.stringify({uid, unsubscribe: channel}));
	}
	console.log('[Account B] Unsubscribed from all channels');
	websocket.close();
} catch (error) {
	console.error(error);
}
