import restify from 'restify';
import catapult from 'catapult-sdk';
import crypto from 'crypto';
import winston from 'winston';
import transactionUtils from './model/transactionUtils';
import spammerOptions from './utils/spammerOptions';
import utils from './utils/spammerUtils';

const address = catapult.model.address;
const serialize = catapult.modelBinary.serialize;
const modelCodec = catapult.plugins.catapultModelSystem.configure(['transfer']).codec;

(function () {
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
	const timer = (function () {
		const startTime = new Date().getTime();
		return { elapsed: () => new Date().getTime() - startTime };
	})();

	function logStats(spammerStats) {
		if (0 !== spammerStats.successful % 10)
			return;

		const throughput = (spammerStats.successful * 1000 / timer.elapsed()).toFixed(2);
		winston.info(`transactions successfully sent so far: ${spammerStats.successful} (${throughput} txes / s)`);
	}

	const predefinedRecipient = (function () {
		const Seed_Private_Key = '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF';
		const recipients = [];
		let curPrivateKey = Seed_Private_Key;
		for (let i = 0; i < options.predefinedRecipients; ++i) {
			const keyPair = catapult.crypto.createKeyPairFromPrivateKeyString(curPrivateKey);
			curPrivateKey = catapult.utils.convert.uint8ToHex(keyPair.publicKey);
			recipients.push(address.publicKeyToAddress(keyPair.publicKey, Mijin_Test_Network));
		}

		return () => recipients[utils.random(options.predefinedRecipients - 1)];
	})();

	function randomRecipient() {
		const keySize = 32;
		const privateKey = crypto.randomBytes(keySize);
		const keyPair = catapult.crypto.createKeyPairFromPrivateKeyString(catapult.utils.convert.uint8ToHex(privateKey));
		return address.publicKeyToAddress(keyPair.publicKey, Mijin_Test_Network);
	}

	const pickKeyPair = (function (privateKeys) {
		const keyPairs = [];
		for (const privateKey of privateKeys)
			keyPairs.push(catapult.crypto.createKeyPairFromPrivateKeyString(privateKey));

		return () => keyPairs[crypto.randomBytes(1)[0] % privateKeys.length];
	})(Private_Keys);

	function createPayload(transfer) {
		return { payload: serialize.toHex(modelCodec, transfer) };
	}

	function sendTransaction() {
		return new Promise(resolve => {
			// don't initiate more transactions than wanted. If a send fails txCounters.initiated will be decremented
			// and thus another transaction will be sent.
			if (txCounters.initiated >= options.total)
				return;

			++txCounters.initiated;
			const keyPair = pickKeyPair();
			const txId = txCounters.initiated;
			const transfer = transactionUtils.createRandomTransfer(
				{ signerPublicKey: keyPair.publicKey, networkId: Mijin_Test_Network,	transferId: txId },
				0 === options.predefinedRecipients ? randomRecipient : predefinedRecipient);
			transactionUtils.signTransaction(modelCodec, keyPair, transfer);
			const txData = createPayload(transfer);
			client.put('/transaction/send', txData, err => {
				if (err) {
					--txCounters.initiated;
					winston.error(`an error occurred while sending the transaction with id ${txId}: ${err.message}`);
				} else {
					++txCounters.successful;
					logStats(txCounters);
				}

				resolve(txCounters.successful);
			});
		});
	}

	if (options.help) {
		winston.info(spammerOptions.usage());
		process.exit();
	}

	(function () {
		const timerId = setInterval(
				() => sendTransaction().then(numSuccessfulTransactions => {
					if (numSuccessfulTransactions < options.total)
						return;

					clearInterval(timerId);
					winston.info('finished');
					process.exit();
				}),
				1000 / options.rate);
	})();
})();
