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
const catapultConnection = require('./catapultConnection');
const errors = require('../server/errors');

const { convert } = catapult.utils;
const { createKeyPairFromPrivateKeyString } = catapult.crypto;

/**
 * Creates a catapult connection service for connecting to catapult servers.
 * This is used for sending data (e.g. transactions) to a server over an authenticated connection.
 * Current implementation only supports maintaining a connection to a single server but can be extended if needed.
 * @param {object} config Service configuration.
 * @param {Function} connectionFactory Factory for creating new net.Socket connections.
 * @param {Function} authPromiseFactory Factory for creating an auth promise around a net.Socket.
 * @param {Function} logger A logging function.
 * @returns {object} The catapult connection service.
 */
module.exports.createConnectionService = (config, connectionFactory, authPromiseFactory, logger = () => {}) => {
	const node = config.apiNode;
	const clientKeyPair = createKeyPairFromPrivateKeyString(config.clientPrivateKey);
	const aliveConnections = {};

	/**
	 * Opens a new connection authenticated to catapult.
	 * @param {boolean} isPersistent Determines whether the new connection should be pooled and kept open for reuse.
	 * @returns {Promise} A promise bound to the creation of the connection.
	 */
	const openAuthenticatedConnection = isPersistent => new Promise((resolve, reject) => {
		logger(`connecting to ${node.host}:${node.port}`);
		const serverSocket = connectionFactory(node.port, node.host);
		const apiNodePublicKey = convert.hexToUint8(node.publicKey);

		serverSocket
			.on('error', err => {
				// capture error, otherwise net default handler will be called
				// default error handler issues reject(), that would go through bootstraper and toRestError().
				// the result might contain information about api node IP and port, because it might be different host,
				// that information shouldn't be available to rest clients.
				logger(`error raised by ${node.host}:${node.port} connection`, err);
			})
			.on('close', () => {
				if (isPersistent)
					delete aliveConnections[node];

				reject(errors.createServiceUnavailableError('connection failed'));
			});

		return authPromiseFactory(serverSocket, clientKeyPair, apiNodePublicKey, logger)
			.then(() => {
				// wrap the socket in a catapult connection and save it
				const serverConnection = catapultConnection.wrap(serverSocket);
				if (isPersistent)
					aliveConnections[node] = serverConnection;

				resolve(serverConnection);
			}, reject);
	});

	return {
		/**
		 * Leases an available connection.
		 * @returns {module:connection/catapultConnection~CatapultConnection} A connection.
		 */
		lease: () => {
			const connection = aliveConnections[node];
			if (connection)
				return Promise.resolve(connection);

			return openAuthenticatedConnection(true);
		},

		/**
		 * Creates a new connection that gets automatically closed after being used.
		 * @returns {module:connection/catapultConnection~CatapultConnection} A connection.
		 */
		singleUse: () => openAuthenticatedConnection(false)
	};
};
