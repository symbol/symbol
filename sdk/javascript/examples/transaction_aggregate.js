#!/usr/bin/env node

//
// Reads utf8 encoded files with names "part<id>.txt".
// Creates aggregate transaction where each files is a message inside embedded transfers.
// Signs aggregate transaction using private key provided in file specified via `--private` switch.
//

import { readContents, readPrivateKey } from './examples_utils.js';
import symbolSdk from '../src/index.js';
import yargs from 'yargs';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

(() => {
	const { SymbolFacade } = symbolSdk.facade;

	const addEmbeddedTransfers = (facade, publicKey) => {
		const textEncoder = new TextEncoder();

		// obtain recipient from publicKey, so direct all transfers to 'self'
		const recipientAddress = facade.network.publicKeyToAddress(publicKey);
		const resourcesDirectory = path.join(path.dirname(fileURLToPath(import.meta.url)), 'resources');

		const filenames = fs.readdirSync(resourcesDirectory).filter(filename => filename.startsWith('part'));
		filenames.sort();

		return filenames.map(filename => {
			const message = readContents(path.join(resourcesDirectory, filename));
			const embeddedTransaction = facade.transactionFactory.createEmbedded({
				type: 'transfer_transaction_v1',
				signerPublicKey: publicKey,
				recipientAddress,
				// note: additional 0 byte at the beginning is added for compatibility with explorer
				// and other tools that treat messages starting with 00 byte as "plain text"
				message: new Uint8Array([0, ...textEncoder.encode(message)])
			});

			console.log(`----> ${filename} length in bytes: ${embeddedTransaction.message.length}`);
			return embeddedTransaction;
		});
	};

	const args = yargs(process.argv.slice(2))
		.demandOption('private', 'path to file with private key')
		.argv;

	const facade = new SymbolFacade('testnet');
	const keyPair = readPrivateKey(args.private);

	const embeddedTransactions = addEmbeddedTransfers(facade, keyPair.publicKey);
	const merkleHash = facade.constructor.hashEmbeddedTransactions(embeddedTransactions);

	const aggregateTransaction = facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v2',
		signerPublicKey: keyPair.publicKey,
		fee: 0n,
		deadline: 1n,
		transactionsHash: merkleHash,
		transactions: embeddedTransactions
	});

	const signature = facade.signTransaction(keyPair, aggregateTransaction);
	facade.transactionFactory.constructor.attachSignature(aggregateTransaction, signature);

	console.log(`Hash: ${facade.hashTransaction(aggregateTransaction)}\n`);
	console.log(aggregateTransaction.toString());
})();
