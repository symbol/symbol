import catapult from 'catapult-sdk';
import fs from 'fs';
import { createConnection } from 'net';
import winston from 'winston';
import modelFormatter from './model/modelFormatter';

const createAuthPromise = catapult.auth.createAuthPromise;
const createKeyPairFromPrivateKeyString = catapult.crypto.createKeyPairFromPrivateKeyString;
const PacketType = catapult.packet.PacketType;
const convert = catapult.utils.convert;

(function () {
	// 1. load configuration
	const config = (function () {
		const configFile = process.argv[2] || '../resources/monitor.json';

		winston.info(`loading config from ${configFile}`);
		return JSON.parse(fs.readFileSync(configFile, 'utf8'));
	})();

	// 2. configure logging
	(function () {
		winston.remove(winston.transports.Console);
		winston.add(winston.transports.Console, config.logging.console);
	})();

	winston.verbose('finished loading monitor config', config);

	// 3. connect to the server
	const serverName = `${config.server.host}:${config.server.port}`;
	const serverSocket = createConnection(config.server.port, config.server.host, () => {
		winston.info(`connected to: ${serverName}`);
	});

	serverSocket.on('close', () => {
		winston.info(`disconnected from: ${serverName}`);
	});

	// 4. authenticate and listen
	const clientKeyPair = createKeyPairFromPrivateKeyString('8D31B712AB28D49591EAF5066E9E967B44507FC19C3D54D742F7B3A255CFF4AB');
	createAuthPromise(serverSocket, clientKeyPair, convert.hexToUint8(config.server.publicKey), winston.verbose)
		.then(parser => {
			parser.onPacket(packet => {
				if (PacketType.pushBlock !== packet.type) {
					winston.warn(`ignoring unknown payload of type ${packet.type} and size ${packet.size}`);
					return;
				}

				winston.info(`received block of size ${packet.size}!`);
				const block = modelFormatter.parseAndFormatBlock(packet.payload);
				winston.info(`parsed block with height ${block.height}`, block);
			});
		});

	// 5. handle signals
	process.on('SIGINT', () => {
		winston.info('SIGINT detected, shutting down monitor');
		serverSocket.destroy();
	});
})();
