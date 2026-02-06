import { PrivateKey } from 'symbol-sdk';
import {
	generateMosaicAliasId,
	NetworkTimestamp,
	SymbolFacade,
	models
} from 'symbol-sdk/symbol';

// OPTION 1

function buildPrefundedMessageTransaction(recipientAddress, message) {
	// Build the embedded message transaction
	const messageTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// Account sending the message
		signerPublicKey: userKeyPair.publicKey,
		recipientAddress,
		message
	});

	// Build the embedded prefund transaction
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

	// Build the wrapper complete aggregate transaction
	const transaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v3',
		// This is the account that will pay for the transaction
		signerPublicKey: userKeyPair.publicKey,
		deadline: timestamp.addHours(2).timestamp,
		transactions: [messageTransaction, prefundTransaction]
	});
	// Calculate total fee, reserving space for a cosignature
	transaction.fee = new models.Amount(
		feeMult * (transaction.size + 104)
	);
	// Update the prefund amount to match the total fee
	prefundTransaction.mosaics[0].amount = transaction.fee;
	// Update the embedded transaction hashes
	transaction.transactionsHash = new models.Hash256(
		facade.static.hashEmbeddedTransactions(
			[messageTransaction, prefundTransaction]
		).bytes
	);

	// Sign the aggregate transaction using the user's signature
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

	return { transaction, jsonPayload };
}

// OPTION 2

function buildSponsoredMessageTransaction(recipientAddress, message) {
	// Build the embedded message transaction
	const messageTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// Account sending the message
		signerPublicKey: userKeyPair.publicKey,
		recipientAddress,
		message
	});

	// Build the embedded filler transaction
	const fillerTransaction = facade.transactionFactory.createEmbedded({
		type: 'transfer_transaction_v1',
		// The application account is both the sender and the recipient
		// and there is no `mosaics` field
		signerPublicKey: appKeyPair.publicKey,
		recipientAddress: facade.network.publicKeyToAddress(
			appKeyPair.publicKey
		)
	});

	// Build the wrapper complete aggregate transaction
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
	transaction.fee = new models.Amount(
		feeMult * (transaction.size + 104)
	);

	// Sign the aggregate transaction using the app's signature
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

	return { transaction, jsonPayload };
}

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
let feeMult;

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
	const medianMult = feeJSON.medianFeeMultiplier;
	const minimumMult = feeJSON.minFeeMultiplier;
	feeMult = Math.max(medianMult, minimumMult);
	console.log('  Fee multiplier:', feeMult);

	// Choose one
	const { transaction, jsonPayload } =
		buildPrefundedMessageTransaction(
			'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
			'Hello world!'
		);
	//const { transaction, jsonPayload } =
	//	buildSponsoredMessageTransaction(
	//		'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
	//		'Hello world!'
	//	);

	console.log('Built transaction:');
	console.log(JSON.stringify(transaction.toJson(), null, 2));


	// Announce the transaction
	const announcePath = '/transactions';
	console.log(`Announcing transaction to ${announcePath}`);
	const response = await fetch(`${NODE_URL}${announcePath}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: jsonPayload
	});
	console.log(`  Response: ${await response.text()}`);

	// Wait for confirmation
	const statusPath =
		`/transactionStatus/${facade.hashTransaction(transaction)}`;
	console.log(`Waiting for confirmation from ${statusPath}`);

	for (let attempt = 0; attempt < 60; ++attempt) {
		await new Promise(resolve => setTimeout(resolve, 1000));
		try {
			const response = await fetch(`${NODE_URL}${statusPath}`);
			if (!response.ok)
				throw new Error(response.statusText);

			const status = await response.json();
			console.log(`  Transaction status: ${status.group}`);
			if (status.group === 'confirmed') {
				console.log(`Transaction confirmed in ${attempt} seconds`);
				break;
			}
			if (status.group === 'failed') {
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
