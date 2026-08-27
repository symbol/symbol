import { PrivateKey } from 'symbol-sdk';
import {
	Address,
	SymbolFacade,
	descriptors,
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

let feeMultiplier;

// OPTION 1
// [>step-1]
function buildPrefundedMessageTransaction(recipientAddress, message) {
	// Build the embedded message transaction [>step-2]
	const messageTransaction =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				new Address(recipientAddress),
				undefined,
				message),
			userKeyPair.publicKey);
	// [<step-2]
	// Build the embedded prefund transaction [>step-3]
	const prefundTransaction =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				facade.network.publicKeyToAddress(userKeyPair.publicKey),
				[
					new descriptors.UnresolvedMosaicDescriptor(
						generateMosaicAliasId('symbol.xym'),
						// To be filled once value is known.
						new models.Amount(0n))
				],
				undefined
			),
			appKeyPair.publicKey);
	// [<step-3]
	// Build the wrapper complete aggregate transaction [>step-4]
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(
				[messageTransaction, prefundTransaction]),
			[messageTransaction, prefundTransaction],
			undefined),
		userKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60,
		1);
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
	const jsonPayload = facade.transactionFactory.static.toJson(
		transaction); // [<step-5]

	return { transaction, jsonPayload };
}
// [<step-1]
// OPTION 2
// [>step-6]
function buildSponsoredMessageTransaction(recipientAddress, message) {
	// Build the embedded message transaction [>step-7]
	const messageTransaction =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				new Address(recipientAddress),
				undefined,
				message),
			userKeyPair.publicKey);
	// [<step-7]
	// Build the embedded filler transaction [>step-8]
	const fillerTransaction =
		facade.createEmbeddedTransactionFromTypedDescriptor(
			new descriptors.TransferTransactionV1Descriptor(
				facade.network.publicKeyToAddress(appKeyPair.publicKey),
				undefined,
				undefined
			),
			appKeyPair.publicKey);
	// [<step-8]
	// Build the wrapper complete aggregate transaction [>step-9]
	const transaction = facade.createTransactionFromTypedDescriptor(
		new descriptors.AggregateCompleteTransactionV3Descriptor(
			facade.static.hashEmbeddedTransactions(
				[messageTransaction, fillerTransaction]),
			[messageTransaction, fillerTransaction],
			undefined),
		appKeyPair.publicKey,
		feeMultiplier,
		2 * 60 * 60,
		1);
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
	const jsonPayload = facade.transactionFactory.static.toJson(
		transaction);
	// [<step-10]
	return { transaction, jsonPayload };
}
// [<step-6]

try {
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
				console.log(
					`Transaction confirmed in ${attempt} seconds`);
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
