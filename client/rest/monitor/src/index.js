/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const catapult = require('catapult-sdk');
const fs = require('fs');
const { createConnection } = require('net');
const winston = require('winston');
const modelFormatter = require('./model/modelFormatter');

const { createAuthPromise } = catapult.auth;
const { createKeyPairFromPrivateKeyString } = catapult.crypto;
const { PacketType } = catapult.packet;
const { convert } = catapult.utils;

(() => {
	// 1. load configuration
	const config = (() => {
		const configFile = process.argv[2] || '../resources/monitor.json';

		winston.info(`loading config from ${configFile}`);
		return JSON.parse(fs.readFileSync(configFile, 'utf8'));
	})();

	// 2. configure logging
	(() => {
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
