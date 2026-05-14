import { PrivateKey } from 'symbol-sdk';
import {
	NetworkTimestamp,
	SymbolFacade,
	generateMosaicAliasId,
	models
} from 'symbol-sdk/symbol';

const NODE_URL = 'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);

const APP_PRIVATE_KEY = process.env.APP_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const appKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(APP_PRIVATE_KEY));
console.log(`App public key: ${appKeyPair.publicKey}`);

const USER_PRIVATE_KEY = process.env.USER_PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000099';
const userKeyPair = new SymbolFacade.KeyPair(
	new PrivateKey(USER_PRIVATE_KEY));
console.log(`User public key: ${userKeyPair.publicKey}`);

const facade = new SymbolFacade('testnet');

let timestamp;
let feeMultiplier;

// OPTION 1
// [>step-1]
function buildPrefundedMessageTransaction(recipientAddress, message) {
	// Build the embedded message transaction [>step-2]
	const messageTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// Account sending the message
		signerPublicKey: userKeyPair.publicKey,
		recipientAddress,
		message
	});
	// [<step-2]
	// Build the embedded prefund transaction [>step-3]
	const prefundTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// Account funding the transaction fee
		signerPublicKey: appKeyPair.publicKey,
		// Account receiving the funds
		recipientAddress: facade.network.publicKeyToAddress(
			userKeyPair.publicKey
		),
		mosaics: [{
			mosaicId: generateMosaicAliasId('symbol.xym'),
			amount: 0 // To be filled once value is known
		}]
	});
	// [<step-3]
	// Build the wrapper complete aggregate transaction [>step-4]
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		// This is the account that will pay for the transaction
		signerPublicKey: userKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		transactions: [messageTransaction, prefundTransaction]
	});
	// Calculate total fee, reserving space for a cosignature
	const cosignatureSize = new models.Cosignature().size;
	transaction.fee = new models.Amount(
		feeMultiplier * (transaction.size + cosignatureSize)
	);
	// Update the prefund amount to match the total fee
	prefundTransaction.mosaics[0].amount = transaction.fee;
	// Update the embedded transaction hashes
	transaction.transactionsHash = new models.Hash256(
		facade.static.hashEmbeddedTransactions(
			[messageTransaction, prefundTransaction]
		).bytes
	);
	// [<step-4]
	// Sign the aggregate transaction using the user's signature [>step-5]
	facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(userKeyPair, transaction)
	);
	// Attach the app's cosignature
	transaction.cosignatures.push(
		facade.cosignTransaction(appKeyPair, transaction)
	);
	// Obtain the payload
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(userKeyPair, transaction)
	);
	// [<step-5]
	return { transaction, jsonPayload };
}
// [<step-1]
// OPTION 2
// [>step-6]
function buildSponsoredMessageTransaction(recipientAddress, message) {
	// Build the embedded message transaction [>step-7]
	const messageTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// Account sending the message
		signerPublicKey: userKeyPair.publicKey,
		recipientAddress,
		message
	});
	// [<step-7]
	// Build the embedded filler transaction [>step-8]
	const fillerTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// The application account is both the sender and the recipient
		// and there is no `mosaics` field
		signerPublicKey: appKeyPair.publicKey,
		recipientAddress: facade.network.publicKeyToAddress(
			appKeyPair.publicKey
		)
	});
	// [<step-8]
	// Build the wrapper complete aggregate transaction [>step-9]
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		// This is the account that will pay for the transaction
		signerPublicKey: appKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		transactionsHash: facade.static.hashEmbeddedTransactions(
			[messageTransaction, fillerTransaction]),
		transactions: [messageTransaction, fillerTransaction]
	});
	// Calculate total fee, reserving space for a cosignature
	const cosignatureSize = new models.Cosignature().size;
	transaction.fee = new models.Amount(
		feeMultiplier * (transaction.size + cosignatureSize)
	);
	// [<step-9]
	// Sign the aggregate transaction using the app's signature [>step-10]
	facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(appKeyPair, transaction)
	);
	// Attach the user's cosignature
	transaction.cosignatures.push(
		facade.cosignTransaction(userKeyPair, transaction)
	);
	// Obtain the payload
	const jsonPayload = facade.transactionFactory.static.attachSignature(
		transaction,
		facade.signTransaction(appKeyPair, transaction)
	);
	// [<step-10]
	return { transaction, jsonPayload };
}
// [<step-6]

try {
	// Fetch current network time
	const timePath = '/node/time';
	console.log('Fetching current network time from', timePath);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJSON = await timeResponse.json();
	timestamp = new NetworkTimestamp(
		timeJSON.communicationTimestamps.receiveTimestamp);
	console.log('  Network time:', timestamp.timestamp,
		'ms since nemesis');

	// Fetch recommended fees
	const feePath = '/network/fees/transaction';
	console.log('Fetching recommended fees from', feePath);
	const feeResponse = await fetch(`${NODE_URL}${feePath}`);
	const feeJSON = await feeResponse.json();
	const medianMultiplier = feeJSON.medianFeeMultiplier;
	const minimumMultiplier = feeJSON.minFeeMultiplier;
	feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
	console.log('  Fee multiplier:', feeMultiplier);

	// Choose one
	const builders = {
		prefunded: buildPrefundedMessageTransaction,
		sponsored: buildSponsoredMessageTransaction
	};
	const { transaction, jsonPayload } = builders.prefunded(
		'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
		'Hello world!'
	);

	console.log('Built transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));

	// Announce the transaction
	const announcePath = '/transactions';
	console.log(`Announcing transaction to ${announcePath}`);
	const announceResponse = await fetch(`${NODE_URL}${announcePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log(`  Response: ${await announceResponse.text()}`);

	// Wait for confirmation
	const statusPath =
		`/transactionStatus/${facade.hashTransaction(transaction)}`;
	console.log(`Waiting for confirmation from ${statusPath}`);

	for (let attempt = 0; 60 > attempt; ++attempt) {
		await new Promise(resolve => { setTimeout(resolve, 1000); });
		try {
			const response = await fetch(`${NODE_URL}${statusPath}`);
			if (!response.ok)
				throw new Error(response.statusText);

			const status = await response.json();
			console.log(`  Transaction status: ${status.group}`);
			if ('confirmed' === status.group) {
				console.log(`Transaction confirmed in ${attempt} seconds`);
				break;
			}
			if ('failed' === status.group) {
				console.log(`Transaction failed: ${status.code}`);
				break;
			}
		} catch (e) {
			console.log(
				`  Transaction status: unknown | Cause: (${e.message})`
			);
		}
	}
} catch (e) {
	console.error(e.message);
}
