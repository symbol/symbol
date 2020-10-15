/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const spammerUtils = require('./model/spammerUtils');
const transactionFactory = require('./model/transactionFactory');
const random = require('./utils/random');
const spammerOptions = require('./utils/spammerOptions');
const catapult = require('catapult-sdk');
const restify = require('restify');
const winston = require('winston');
const crypto = require('crypto');

const { address } = catapult.model;
const { serialize, transactionExtensions } = catapult.modelBinary;
const modelCodec = catapult.plugins.catapultModelSystem.configure(['transfer', 'aggregate'], {}).codec;
const { uint64 } = catapult.utils;

(() => {
	const Mijin_Test_Network = catapult.model.networkInfo.networks.mijinTest.id;
	const options = spammerOptions.options();

	const client = restify.createJsonClient({
		url: `http://${options.address}:${options.port}`,
		connectTimeout: 1000
	});

	const Private_Keys = [
		'8473645728B15F007385CE2889D198D26369D2806DCDED4A9B219FD0DE23A505',
		'BBC3E5BE46A953070B0B9636E386C2006DA9EA8840596B601D4A1B92A9F93330',
		'46C83EE87DAB6588DD82D1059140D3E5A7FAFF78C3A0C4CE4802486D71C69E40',
		'FA19F42DDD6E757B3A2E39E75A7487F8FEC19C0E872153EC0EFD9AC2E5A84E58'
	];

	const txCounters = { initiated: 0, successful: 0 };
	const timer = (() => {
		const startTime = new Date().getTime();
		return { elapsed: () => new Date().getTime() - startTime };
	})();

	const logStats = spammerStats => {
		if (0 !== spammerStats.successful % 10)
			return;

		const throughput = (spammerStats.successful * 1000 / timer.elapsed()).toFixed(2);
		winston.info(`transactions successfully sent so far: ${spammerStats.successful} (${throughput} txes / s)`);
	};

	const predefinedRecipient = (() => {
		const Seed_Private_Key = '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF';
		const recipients = [];
		let curPrivateKey = Seed_Private_Key;
		for (let i = 0; i < options.predefinedRecipients; ++i) {
			const keyPair = catapult.crypto.createKeyPairFromPrivateKeyString(curPrivateKey);
			curPrivateKey = catapult.utils.convert.uint8ToHex(keyPair.publicKey);
			recipients.push(address.publicKeyToAddress(keyPair.publicKey, Mijin_Test_Network));
		}

		return () => recipients[random.uint32(options.predefinedRecipients - 1)];
	})();

	const randomRecipient = () => {
		const keySize = 32;
		const privateKey = crypto.randomBytes(keySize);
		const keyPair = catapult.crypto.createKeyPairFromPrivateKeyString(catapult.utils.convert.uint8ToHex(privateKey));
		return address.publicKeyToAddress(keyPair.publicKey, Mijin_Test_Network);
	};

	const pickKeyPair = (privateKeys => {
		const keyPairs = privateKeys.map(privateKey => catapult.crypto.createKeyPairFromPrivateKeyString(privateKey));
		return () => keyPairs[crypto.randomBytes(1)[0] % privateKeys.length];
	})(Private_Keys);

	const createPayload = transfer => ({ payload: serialize.toHex(modelCodec, transfer) });

	const prepareTransferTransaction = txId => {
		const keyPair = pickKeyPair();
		const transfer = transactionFactory.createRandomTransfer(
			{ signerPublicKey: keyPair.publicKey, networkId: Mijin_Test_Network, transferId: txId },
			0 === options.predefinedRecipients ? randomRecipient : predefinedRecipient
		);
		transactionExtensions.sign(modelCodec, keyPair, transfer);
		return transfer;
	};

	const sendTransaction = createAndPrepareTransaction => new Promise(resolve => {
		// don't initiate more transactions than wanted. If a send fails txCounters.initiated will be decremented
		// and thus another transaction will be sent.
		if (txCounters.initiated >= options.total)
			return;

		++txCounters.initiated;
		const transaction = createAndPrepareTransaction(txCounters.initiated);

		const txData = createPayload(transaction);
		client.put('/transaction', txData, err => {
			if (err) {
				winston.error(`an error occurred while sending the transaction with id ${txCounters.initiated}`, err);
				--txCounters.initiated;
			} else {
				++txCounters.successful;
				logStats(txCounters);
			}

			resolve(txCounters.successful);
		});
	});

	const randomKeyPair = () => {
		const keySize = 32;
		const privateKey = crypto.randomBytes(keySize);
		return catapult.crypto.createKeyPairFromPrivateKeyString(catapult.utils.convert.uint8ToHex(privateKey));
	};

	const createTransfer = (signerPublicKey, recipientAddress, transferId, amount) => {
		const transfer = transactionFactory.createRandomTransfer(
			{ signerPublicKey, networkId: Mijin_Test_Network, transferId },
			() => recipientAddress
		);

		transfer.mosaics[0].amount = amount;
		return transfer;
	};

	const prepareAggregateTransaction = txId => {
		const keyPairSender = pickKeyPair();
		const numProxies = 1 + random.uint32(5);
		const keyPairs = Array.from(Array(numProxies), randomKeyPair);
		keyPairs.unshift(keyPairSender);

		const addresses = keyPairs.map(keyPair => address.publicKeyToAddress(keyPair.publicKey, Mijin_Test_Network));
		const transfers = [];

		for (let i = 0; i < numProxies; ++i) {
			transfers.push(createTransfer(
				keyPairs[i].publicKey,
				addresses[i + 1],
				txId,
				uint64.fromUint(((numProxies + 1 - i) * 1000000) + random.uint32(1000000))
			));
		}

		transfers.push(createTransfer(
			keyPairs[keyPairs.length - 1].publicKey,
			randomRecipient(),
			txId,
			uint64.fromUint(random.uint32(1000000))
		));

		const aggregate = transactionFactory.createAggregateTransaction(
			{ signerPublicKey: keyPairSender.publicKey, networkId: Mijin_Test_Network },
			transfers
		);

		keyPairs.shift();
		spammerUtils.signAndStitchAggregateTransaction(modelCodec, keyPairSender, keyPairs, aggregate);
		return aggregate;
	};

	if (options.help) {
		winston.info(spammerOptions.usage());
		process.exit();
	}

	(() => {
		const transactionFactories = {
			transfer: prepareTransferTransaction,
			aggregate: prepareAggregateTransaction
		};

		const mode = options.mode in transactionFactories ? options.mode : 'transfer';
		const timerId = setInterval(
			() => sendTransaction(transactionFactories[mode]).then(numSuccessfulTransactions => {
				if (numSuccessfulTransactions < options.total)
					return;

				clearInterval(timerId);
				winston.info('finished');
				process.exit();
			}),
			1000 / options.rate
		);
	})();
})();
