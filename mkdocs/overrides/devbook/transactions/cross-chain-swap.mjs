import { PrivateKey, Hash256 } from 'symbol-sdk';
import {
	SymbolFacade,
	NetworkTimestamp,
	models,
	generateMosaicAliasId
} from 'symbol-sdk/symbol';
import { createHash, randomBytes } from 'crypto';
import { ethers } from 'ethers';

const SYMBOL_NODE_URL = process.env.SYMBOL_NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using Symbol node', SYMBOL_NODE_URL);

const ETH_RPC_URL = process.env.ETH_RPC_URL || 'https://ethereum-sepolia-rpc.publicnode.com';
console.log('Using Ethereum RPC', ETH_RPC_URL);

// Ethereum HTLC contract on Sepolia
const HTLC_ADDRESS = '0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B';
const HTLC_ABI = [
	'function newContract(address, bytes32, uint) '
		+ 'external payable returns (bytes32)',
	'function withdraw(bytes32, bytes) '
		+ 'external returns (bool)',
	'function getContract(bytes32) external view '
		+ 'returns (address sender, address receiver, '
		+ 'uint amount, bytes32 hashlock, '
		+ 'uint timelock, bool withdrawn, '
		+ 'bool refunded, bytes preimage)',
	'event LogHTLCNew(bytes32 indexed contractId, '
		+ 'address indexed sender, '
		+ 'address indexed receiver, uint amount, '
		+ 'bytes32 hashlock, uint timelock)'
];

// Helper function to fetch current Symbol network time
async function getNetworkTime() {
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse =
		await fetch(`${SYMBOL_NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	const timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log('  Network time:', timestamp.timestamp,
		'ms since nemesis');
	return timestamp;
}

// Helper function to fetch recommended Symbol fee multiplier
async function getFeeMultiplier() {
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse =
		await fetch(`${SYMBOL_NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	const feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);
	return feeMult;
}

// Helper function to announce a Symbol transaction
async function announceTransaction(payload, endpoint, label) {
	console.log(`Announcing ${label} to ${endpoint}`);
	const response = await fetch(
		`${SYMBOL_NODE_URL}${endpoint}`, {
			method: 'PUT',
			headers: { 'Content-Type': 'application/json' },
			body: payload
		});
	console.log('  Response:', await response.text());
}

// Helper function to wait for Symbol transaction status
async function waitForStatus(hash, expectedStatus, label) {
	console.log(
		`Waiting for ${label} to reach ${expectedStatus} status...`);
	let attempts = 0;
	const maxAttempts = 60;

	while (attempts < maxAttempts) {
		try {
			const url = `${SYMBOL_NODE_URL}/transactionStatus/${hash}`;
			const response = await fetch(url);

			if (!response.ok) {
				const error = new Error(
					`HTTP ${response.status}: ${response.statusText}`);
				error.status = response.status;
				throw error;
			}

			const status = await response.json();
			console.log('  Transaction status:', status.group);

			if (status.group === 'failed') {
				throw new Error(`${label} failed: ${status.code}`);
			}

			if (status.group === expectedStatus) {
				console.log(
					`${label} ${expectedStatus} in ${attempts} seconds`
				);
				return;
			}
		} catch (error) {
			if (error.status === 404) {
				console.log('  Transaction status: not yet available');
			} else {
				throw error;
			}
		}

		attempts++;
		await new Promise(resolve => setTimeout(resolve, 1000));
	}

	throw new Error(
		`${label} not ${expectedStatus} after ${maxAttempts} attempts`);
}

// Symbol accounts
const facade = new SymbolFacade('testnet');

// Alice (creates the ETH lock, claims XYM on Symbol)
const ALICE_PRIVATE_KEY = process.env.ALICE_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const aliceKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(ALICE_PRIVATE_KEY));
const aliceAddress = facade.network.publicKeyToAddress(
	aliceKeyPair.publicKey);
console.log('Alice Symbol address:', aliceAddress.toString());

// Bob (creates the XYM lock, claims ETH on Ethereum)
const BOB_PRIVATE_KEY = process.env.BOB_PRIVATE_KEY ||
	'1111111111111111111111111111111111111111111111111111111111111111';
const bobKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(BOB_PRIVATE_KEY));
const bobAddress = facade.network.publicKeyToAddress(
	bobKeyPair.publicKey);
console.log('Bob Symbol address:', bobAddress.toString());

// Ethereum accounts
const ethProvider = new ethers.JsonRpcProvider(ETH_RPC_URL);

const ALICE_ETH_KEY = process.env.ALICE_ETH_PRIVATE_KEY ||
	'0xa73276699ba72dc7b5c9d08deaf8cd88eec33c866341b120304432b89586d45d';
const aliceEthWallet = new ethers.Wallet(ALICE_ETH_KEY, ethProvider);
console.log('Alice ETH address:', aliceEthWallet.address);

const BOB_ETH_KEY = process.env.BOB_ETH_PRIVATE_KEY ||
	'0x8e85561005f27d926af79a7ce3e76e75108a09ff2ab78bf65b5578d2e5d509bf';
const bobEthWallet = new ethers.Wallet(BOB_ETH_KEY, ethProvider);
console.log('Bob ETH address:', bobEthWallet.address);

try {
	// --- Alice: Generate proof and hashlock ---
	console.log('\n--- Alice: Generate proof and hashlock ---');

	const proof = randomBytes(32);
	console.log('Proof (hex):', proof.toString('hex'));

	const firstHash = createHash('sha256').update(proof).digest();
	const secret = createHash('sha256').update(firstHash).digest();
	console.log('Secret (double SHA-256):', secret.toString('hex'));

	// --- Alice: Lock ETH on Ethereum ---
	console.log('\n--- Alice: Lock ETH on Ethereum ---');

	const htlcAsAlice = new ethers.Contract(
		HTLC_ADDRESS, HTLC_ABI, aliceEthWallet);

	const timelock = Math.floor(Date.now() / 1000) + 72 * 60 * 60;
	console.log('Ethereum timelock (Unix):', timelock);

	const lockTx = await htlcAsAlice.newContract(
		bobEthWallet.address,
		'0x' + secret.toString('hex'),
		timelock,
		{ value: ethers.parseEther('0.01') }
	);
	console.log('Lock TX hash:', lockTx.hash);

	const lockReceipt = await lockTx.wait();
	console.log('Lock confirmed in block', lockReceipt.blockNumber);

	// Extract the contractId from the LogHTLCNew event
	const contractId = lockReceipt.logs[0].topics[1];
	console.log('HTLC contract ID:', contractId);

	// --- Bob: Create secret lock on Symbol ---
	console.log('\n--- Bob: Create secret lock on Symbol ---');

	// Bob queries the Ethereum contract to get the hashlock
	const htlcAsBob = new ethers.Contract(
		HTLC_ADDRESS, HTLC_ABI, bobEthWallet);
	const contractInfo = await htlcAsBob.getContract(contractId);
	const hashlock = contractInfo.hashlock.slice(2); // strip 0x prefix
	console.log('Hashlock from chain:', hashlock);

	const lockDuration = 5760n; // ~48h at 30s blocks
	console.log('Lock duration:', lockDuration.toString(), 'blocks');

	const secretLockTransaction =
		facade.transactionFactory.create({
			type: 'secret_lock_transaction_v1',
			signerPublicKey: bobKeyPair.publicKey.toString(),
			deadline: (await getNetworkTime()).addHours(2).timestamp,
			recipientAddress: aliceAddress.toString(),
			mosaic: {
				mosaicId: generateMosaicAliasId('symbol.xym'),
				amount: 1_000000n // 1 XYM
			},
			duration: lockDuration,
			secret: hashlock,
			hashAlgorithm: 'hash_256'
		});
	secretLockTransaction.fee = new models.Amount(
		(await getFeeMultiplier()) * secretLockTransaction.size);

	// Sign and announce
	const lockSignature = facade.signTransaction(
		bobKeyPair, secretLockTransaction);
	const lockPayload = facade.transactionFactory.static.attachSignature(
		secretLockTransaction, lockSignature);

	console.log('Built secret lock transaction:');
	console.dir(secretLockTransaction.toJson(), { colors: true });

	const lockHash = facade.hashTransaction(
		secretLockTransaction).toString();
	console.log('Secret lock transaction hash:', lockHash);
	await announceTransaction(lockPayload, '/transactions', 'secret lock');
	await waitForStatus(lockHash, 'confirmed', 'Secret lock');

	// --- Alice: Claim XYM on Symbol ---
	console.log('\n--- Alice: Claim XYM on Symbol ---');

	const secretProofTransaction =
		facade.transactionFactory.create({
			type: 'secret_proof_transaction_v1',
			signerPublicKey:
				aliceKeyPair.publicKey.toString(),
			deadline: (await getNetworkTime()).addHours(2).timestamp,
			recipientAddress: aliceAddress.toString(),
			secret: hashlock,
			hashAlgorithm: 'hash_256',
			proof: proof
		});
	secretProofTransaction.fee = new models.Amount(
		(await getFeeMultiplier()) * secretProofTransaction.size);

	// Sign and announce
	const proofSignature = facade.signTransaction(
		aliceKeyPair, secretProofTransaction);
	const proofPayload = facade.transactionFactory.static.attachSignature(
		secretProofTransaction, proofSignature);

	console.log('Built secret proof transaction:');
	console.dir(secretProofTransaction.toJson(), { colors: true });

	const proofHash = facade.hashTransaction(
		secretProofTransaction).toString();
	console.log('Secret proof transaction hash:', proofHash);
	await announceTransaction(
		proofPayload, '/transactions', 'secret proof');
	await waitForStatus(proofHash, 'confirmed', 'Secret proof');

	// --- Bob: Withdraw ETH on Ethereum ---
	console.log('\n--- Bob: Withdraw ETH on Ethereum ---');

	// Retrieve the proof from the confirmed transaction
	const txPath = `/transactions/confirmed/${proofHash}`;
	console.log('Fetching proof from', txPath);
	const txResponse = await fetch(`${SYMBOL_NODE_URL}${txPath}`);
	const txJSON = await txResponse.json();
	const revealedProof = Buffer.from(txJSON.transaction.proof, 'hex');
	console.log('Proof from chain:', revealedProof.toString('hex'));

	const withdrawTx = await htlcAsBob.withdraw(contractId, revealedProof);
	console.log('Withdraw TX hash:', withdrawTx.hash);

	const withdrawReceipt = await withdrawTx.wait();
	console.log('Withdraw confirmed in block',
		withdrawReceipt.blockNumber);

	console.log('\n--- Cross-chain swap complete ---');
} catch (e) {
	console.error(e.message);
}
