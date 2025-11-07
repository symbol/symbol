import { PrivateKey } from 'symbol-sdk';
import {
	generateMosaicAliasId,
	models,
	NetworkTimestamp,
	SymbolFacade
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://001-sai-dual.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Account A (initiates the aggregate and sends XYM)
const ACCOUNT_A_PRIVATE_KEY = process.env.ACCOUNT_A_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const accountAKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_A_PRIVATE_KEY));

// Account B (sends custom mosaic back to Account A)
const ACCOUNT_B_PRIVATE_KEY = process.env.ACCOUNT_B_PRIVATE_KEY || (
	'1111111111111111111111111111111111111111111111111111111111111111');
const accountBKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_B_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
console.log('Account A:', facade.network.publicKeyToAddress(
	accountAKeyPair.publicKey).toString());
console.log('Account B:', facade.network.publicKeyToAddress(
	accountBKeyPair.publicKey).toString());

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

	const accountAAddress = facade.network.publicKeyToAddress(
		accountAKeyPair.publicKey);
	const accountBAddress = facade.network.publicKeyToAddress(
		accountBKeyPair.publicKey);

	// Account A sends 10 XYM to Account B
	const embeddedTransaction1 = facade.transactionFactory
		.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		recipientAddress: accountBAddress.toString(),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 10_000_000n  // 10 XYM (divisibility = 6)
		}]
	});

	// Account B sends 1 custom token to Account A
	const customMosaicId = 0x6D1314BE751B62C2n;
	const embeddedTransaction2 = facade.transactionFactory
		.createEmbedded({
		type: 'transfer_transaction_v1',
		signerPublicKey: accountBKeyPair.publicKey.toString(),
		recipientAddress: accountAAddress.toString(),
		mosaics: [{
			mosaicId: customMosaicId,
			amount: 1n  // 1 custom mosaic (divisibility = 0)
		}]
	});

	// Build the aggregate bonded transaction
	const embeddedTransactions = [
		embeddedTransaction1, embeddedTransaction2];
	const bondedTransaction = facade.transactionFactory.create({
		type: 'aggregate_bonded_transaction_v3',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			embeddedTransactions),
		transactions: embeddedTransactions
	});
	// Reserve space for one cosignature (104 bytes each)
	// and calculate fee for the final transaction size
	bondedTransaction.fee = new models.Amount(
		feeMult * (bondedTransaction.size + 104)
	);
	console.log('Built aggregate bonded transaction:');
	console.log(JSON.stringify(bondedTransaction.toJson(), null, 2));

	// Sign the bonded transaction
	const bondedSignature = facade.signTransaction(
		accountAKeyPair, bondedTransaction);
	const bondedJsonPayload = facade.transactionFactory.static
		.attachSignature(bondedTransaction, bondedSignature);
	const bondedHash = facade.hashTransaction(
		bondedTransaction).toString();
	console.log('Bonded transaction hash:', bondedHash);

	// Create hash lock transaction
	console.log('Creating hash lock transaction...');
	const hashLock = facade.transactionFactory.create({
		type: 'hash_lock_transaction_v1',
		signerPublicKey: accountAKeyPair.publicKey.toString(),
		deadline: timestamp.addHours(2).timestamp,
		mosaic: {
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 10_000_000n  // 10 XYM deposit
		},
		duration: 100n,  // Lock duration in blocks
		hash: bondedHash
	});
	hashLock.fee = new models.Amount(feeMult * hashLock.size);

	// Sign and announce hash lock
	const hashLockSignature = facade.signTransaction(
		accountAKeyPair, hashLock);
	const hashLockPayload = facade.transactionFactory.static
		.attachSignature(hashLock, hashLockSignature);
	const hashLockHash = facade.hashTransaction(hashLock)
		.toString();
	console.log('Hash lock transaction hash:', hashLockHash);

	const announcePath = '/transactions';
	console.log('Announcing hash lock to', announcePath);
	const announceResponse = await fetch(`${NODE_URL}${announcePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: hashLockPayload
	});
	console.log('  Response:', await announceResponse.text());

	// Wait for hash lock confirmation
	console.log('Waiting for hash lock confirmation...');
	const hashLockStatusPath = `/transactionStatus/${hashLockHash}`;
	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const statusResponse = await fetch(
				`${NODE_URL}${hashLockStatusPath}`);
			if (statusResponse.ok) {
				const status = await statusResponse.json();
				if (status.group === 'confirmed') {
					console.log('  Hash lock confirmed');
					break;
				}
			}
		} catch (e) {
			console.log('  Hash lock status: unknown | Cause:',
				e.message);
		}
	}

	// Announce bonded transaction
	const partialPath = '/transactions/partial';
	console.log('Announcing bonded transaction to', partialPath);
	const partialResponse = await fetch(`${NODE_URL}${partialPath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: bondedJsonPayload
	});
	console.log('  Response:', await partialResponse.text());

	// Wait for transaction to reach partial status
	console.log(
		'Waiting for bonded transaction to reach partial status...'
	);
	const statusPath = `/transactionStatus/${bondedHash}`;
	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const statusResponse = await fetch(
				`${NODE_URL}${statusPath}`);
			if (statusResponse.ok) {
				const status = await statusResponse.json();
				if (status.group === 'partial') {
					console.log(
						'  Transaction is partial, ready for cosignatures'
					);
					break;
				}
			}
		} catch (e) {
			console.log('  Transaction status: unknown | Cause:',
				e.message);
		}
	}

	// Submit Account B's cosignature
	console.log('Submitting Account B\'s cosignature...');
	const cosignaturePath = '/transactions/cosignature';
	const cosignature = facade.cosignTransaction(
		accountBKeyPair, bondedTransaction, true);
	const cosignaturePayload = JSON.stringify({
		version: cosignature.version.toString(),
		signerPublicKey: cosignature.signerPublicKey.toString(),
		signature: cosignature.signature.toString(),
		parentHash: cosignature.parentHash.toString()
	});

	const cosigResponse = await fetch(`${NODE_URL}${cosignaturePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: cosignaturePayload
	});
	console.log(
		'  Cosignature from Account B:',
		await cosigResponse.text()
	);

	// Wait for final confirmation
	console.log('Waiting for bonded transaction confirmation...');

	for (let attempt = 0; attempt < 60; attempt++) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const statusResponse = await fetch(
				`${NODE_URL}${statusPath}`);
			const status = await statusResponse.json();
			console.log('  Transaction status:', status.group);
			if (status.group === 'confirmed') {
				console.log(
					'Transaction confirmed in', attempt, 'seconds'
				);
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
	}
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
