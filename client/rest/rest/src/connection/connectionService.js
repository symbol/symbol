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

const catapultConnection = require('./catapultConnection');
const errors = require('../server/errors');
const tls = require('tls');

/**
 * Creates a catapult connection service for connecting to catapult servers.
 * This is used for sending data (e.g. transactions) to a server over an authenticated connection.
 * Current implementation only supports maintaining a connection to a single server but can be extended if needed.
 * @param {object} config Service configuration.
 * @param {Function} logger A logging function.
 * @returns {object} Catapult connection service.
 */
module.exports.createConnectionService = (config, logger = () => {}) => {
	const node = config.apiNode;
	const aliveConnections = {};
	// the connection is not persisted until authentication is done, and this may take a while,
	// to avoid duplicate connections those are cached in the following object
	const authorizingConnectionPromises = {};

	/**
	 * Opens a new connection authenticated to catapult.
	 * @param {boolean} isPersistent Determines whether the new connection should be pooled and kept open for reuse.
	 * @returns {Promise} A promise bound to the creation of the connection.
	 */
	const openAuthorizedConnection = isPersistent => {
		logger(`connecting to ${node.host}:${node.port}`);

		const contextOptions = {
			minVersion: 'TLSv1.3',
			key: config.key,
			cert: config.certificate,
			ca: config.caCertificate
			// sigalgs: 'ed25519'
		};
		let secureContext;
		try {
			secureContext = tls.createSecureContext(contextOptions);
		} catch (error) {
			logger('an error occurred with the provided TLS key and certificates before trying to establish any connection to the server');
			throw error;
		}

		const connectionOptions = {
			host: node.host,
			port: node.port,
			secureContext,
			// skip hostname checks since this is not a web-https case
			checkServerIdentity: () => undefined
		};

		const connectionPromise = new Promise((resolve, reject) => {
			const serverSocket = tls.connect(connectionOptions);

			serverSocket
				.once('secureConnect', () => {
					if (serverSocket.authorized) {
						// wrap the socket in a catapult connection and save it
						const serverConnection = catapultConnection.wrap(serverSocket);
						if (isPersistent) {
							aliveConnections[node] = serverConnection;
							delete authorizingConnectionPromises[node];
						}
						// return, and resolve the connection for possible queued connections on `authorizingConnectionPromises`
						resolve(serverConnection);
						return serverConnection;
					}

					logger(`failed while connecting to the node ${node.host}:${node.port}`, serverSocket.authorizationError);
					reject(serverSocket.authorizationError);
					throw serverSocket.authorizationError;
				})
				.on('error', err => {
					// capture error, otherwise net default handler will be called
					// default error handler issues reject(), that would go through bootstraper and toRestError().
					// the result might contain information about api node IP and port, because it might be different host,
					// that information shouldn't be available to rest clients.
					logger(`error raised by ${node.host}:${node.port} connection: `, err);
				})
				.on('close', () => {
					if (isPersistent)
						delete aliveConnections[node];

					reject(errors.createServiceUnavailableError('connection failed'));
				});
		});

		if (isPersistent)
			authorizingConnectionPromises[node] = connectionPromise;

		return connectionPromise;
	};

	return {
		/**
		 * Leases an available connection.
		 * @returns {module:connection/catapultConnection~CatapultConnection} A connection.
		 */
		lease: () => {
			const authorizingPromise = authorizingConnectionPromises[node];
			if (authorizingPromise)
				return authorizingPromise;

			const connection = aliveConnections[node];
			if (connection)
				return Promise.resolve(connection);

			return openAuthorizedConnection(true);
		},

		/**
		 * Creates a new connection that gets automatically closed after being used.
		 * @returns {module:connection/catapultConnection~CatapultConnection} A connection.
		 */
		singleUse: () => openAuthorizedConnection(false)
	};
};
