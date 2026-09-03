import { Hash256, PrivateKey } from 'symbol-sdk';
import {
	SymbolFacade,
	descriptors,
	generateMosaicAliasId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Helper function to announce transaction
async function announceTransaction(payload, endpoint, label) {
	console.log(`Announcing ${label} to ${endpoint}`);
	const response = await fetch(`${NODE_URL}${endpoint}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: payload
	});
	console.log('  Response:', await response.text());
}

// Helper function to wait for transaction status
async function waitForStatus(hash, expectedStatus, label) {
	console.log(
		`Waiting for ${label} to reach ${expectedStatus} status...`);
	let attempts = 0;
	const maxAttempts = 60;

	while (attempts < maxAttempts) {
		try {
			const url = `${NODE_URL}/transactionStatus/${hash}`;
			const response = await fetch(url);

			if (!response.ok) {
				const error = new Error(
					`HTTP ${response.status}: ${response.statusText}`);
				error.status = response.status;
				throw error;
			}

			const status = await response.json();

			console.log('  Transaction status:', status.group);

			if ('failed' === status.group)
				throw new Error(`${label} failed: ${status.code}`);

			if (status.group === expectedStatus) {
				console.log(
					`${label} ${expectedStatus} in ${attempts} seconds`
				);
				return;
			}
		} catch (error) {
			if (404 === error.status)
				console.log('  Transaction status: not yet available');
			else
				throw error;
		}

		attempts++;
		await new Promise(resolve => { setTimeout(resolve, 1000); });
	}

	throw new Error(
		`${label} not ${expectedStatus} after ${maxAttempts} attempts`
	);
}

// Account A (initiates the aggregate tx and sends XYM to Account B) [>step-1]
const ACCOUNT_A_PRIVATE_KEY = process.env.ACCOUNT_A_PRIVATE_KEY || (
	'0000000000000000000000000000000000000000000000000000000000000000');
const accountAKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_A_PRIVATE_KEY));

// Account B (sends custom mosaic to Account A)
const ACCOUNT_B_PRIVATE_KEY = process.env.ACCOUNT_B_PRIVATE_KEY || (
	'1111111111111111111111111111111111111111111111111111111111111111');
const accountBKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ACCOUNT_B_PRIVATE_KEY));

const facade = new SymbolFacade('testnet');
const accountAAddress = facade.network.publicKeyToAddress(
	accountAKeyPair.publicKey);
const accountBAddress = facade.network.publicKeyToAddress(
	accountBKeyPair.publicKey);
console.log('Account A:', accountAAddress.toString());
console.log('Account B:', accountBAddress.toString());
// [<step-1]
try {
	// Fetch recommended fees [>step-2]
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	const feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier); // [<step-2]

	// Embedded tx 1: Account A transfers 10 XYM to Account B [>step-3]
	const embeddedTransaction1 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				accountBAddress,
				[
					new descriptors.UnresolvedMosaicDescriptor(
						generateMosaicAliasId('symbol.xym'),
						new models.Amount(10_000_000n)) // 10 XYM
				],
				undefined),
			accountAKeyPair.publicKey);

	// Embedded tx 2: Account B transfers 1 custom mosaic to Account A
	const customMosaicId = 0x6D1314BE751B62C2n;
	const embeddedTransaction2 =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				accountAAddress,
				[
					new descriptors.UnresolvedMosaicDescriptor(
						customMosaicId,
						new models.Amount(1n)) // 1 custom mosaic
				],
				undefined),
			accountBKeyPair.publicKey); // [<step-3]

	// Build the bonded aggregate transaction [>step-4]
	const embeddedTransactions = [
		embeddedTransaction1, embeddedTransaction2];
	const bondedTransaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateBondedTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(embeddedTransactions),
			embeddedTransactions,
			undefined),
		accountAKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60,
		1);
	console.log('Built aggregate without signatures:');
	console.log(JSON.stringify(bondedTransaction.toJson(), null, 2));
	// [<step-4]
	// --- ACCOUNT A (Initiator) --- [>step-5]
	// Sign the bonded aggregate transaction
	console.log('[Account A] Signing the bonded aggregate...');
	const bondedSignature = facade.signTransaction(
		accountAKeyPair, bondedTransaction);
	const bondedJsonPayload = facade.transactionFactory.static
		.attachSignature(bondedTransaction, bondedSignature);
	const bondedHash = facade.hashTransaction(
		bondedTransaction).toString();
	console.log('Bonded aggregate transaction hash:', bondedHash);
	// [<step-5]
	// Create hash lock transaction [>step-6]
	console.log('Creating hash lock transaction...');
	const hashLock = facade.createTransactionFromTypedDescriptor(
		new descriptors.HashLockTransactionV1Descriptor(
			new descriptors.UnresolvedMosaicDescriptor(
				generateMosaicAliasId('symbol.xym'),
				new models.Amount(10_000_000n)), // 10 XYM deposit
			new models.BlockDuration(100n),
			new Hash256(bondedHash)),
		accountAKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60);

	// Sign hash lock
	console.log('[Account A] Signing the hash lock...');
	const hashLockSignature = facade.signTransaction(
		accountAKeyPair, hashLock);
	const hashLockPayload = facade.transactionFactory.static
		.attachSignature(hashLock, hashLockSignature);
	const hashLockHash = facade.hashTransaction(hashLock)
		.toString();
	console.log('Hash lock transaction hash:', hashLockHash);

	// Announce hash lock and wait for confirmation
	await announceTransaction(
		hashLockPayload, '/transactions', 'Hash lock'
	);
	await waitForStatus(hashLockHash, 'confirmed', 'Hash lock');
	// [<step-6]
	// Announce bonded aggregate and wait for partial status // [>step-7]
	await announceTransaction(
		bondedJsonPayload, '/transactions/partial',
		'Bonded aggregate transaction'
	);
	await waitForStatus(
		bondedHash, 'partial', 'Bonded aggregate transaction'
	); // [<step-7]

	// --- ACCOUNT B (Cosigner) --- [>step-8]
	// Retrieves partial transactions waiting for signature
	const partialPath =
		`/transactions/partial?address=${accountBAddress}`;
	console.log(
		'[Account B] Checking for partial transactions from ' +
		'/transactions/partial'
	);
	const partialResponse = await fetch(`${NODE_URL}${partialPath}`);
	const partialTxs = await partialResponse.json();
	if (!partialTxs.data || 0 === partialTxs.data.length)
		throw new Error('No partial transactions found');

	console.log(
		`Found ${partialTxs.data.length} partial transaction(s)`);

	// Find the transaction matching the expected hash
	const found = partialTxs.data.some(
		tx => tx.meta.hash === bondedHash
	);
	if (!found) {
		throw new Error(
			`Expected transaction ${bondedHash} not found in ` +
			'partial transactions'
		);
	}
	console.log(`Found matching transaction: ${bondedHash}`);
	// [<step-8]
	// Fetch full transaction details using the hash [>step-9]
	const detailPath = `/transactions/partial/${bondedHash}`;
	const detailResponse = await fetch(`${NODE_URL}${detailPath}`);
	const partialTxJson = await detailResponse.json();

	// Verify transaction content before cosigning
	const txData = partialTxJson.transaction;
	console.log('[Account B] Verifying transaction: ' +
		`${txData.transactions.length} embedded transactions`
	); // [<step-9]

	// Submit Account B's cosignature using the transaction hash [>step-10]
	const cosignaturePath = '/transactions/cosignature';
	console.log('[Account B] Cosigning the bonded aggregate...');
	const cosignature = SymbolFacade.cosignTransactionHash(
		accountBKeyPair, new Hash256(bondedHash), true
	);
	const cosignaturePayload = JSON.stringify({
		version: cosignature.version.toString(),
		signerPublicKey: cosignature.signerPublicKey.toString(),
		signature: cosignature.signature.toString(),
		parentHash: cosignature.parentHash.toString()
	});

	// Announce cosignature
	await announceTransaction(
		cosignaturePayload, cosignaturePath, 'cosignature'
	); // [<step-10]

	// Wait for final confirmation [>step-11]
	await waitForStatus(
		new Hash256(bondedHash), 'confirmed',
		'Bonded aggregate transaction'
	); // [<step-11]
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
}
