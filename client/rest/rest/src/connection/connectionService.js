import catapult from 'catapult-sdk';
import catapultConnection from './catapultConnection';
import errors from '../server/errors';

const convert = catapult.utils.convert;
const createKeyPairFromPrivateKeyString = catapult.crypto.createKeyPairFromPrivateKeyString;

/**
 * Creates a catapult connection service.
 * @param {object} config Service configuration.
 * @param {Function} connectionFactory Factory for creating new net.Socket connections.
 * @param {Function} authPromiseFactory Factory for creating an auth promise around a net.Socket.
 * @param {Function} logger A logging function.
 * @returns {object} The catapult connection service.
 */
export default function createConnectionService(config, connectionFactory, authPromiseFactory, logger = () => {}) {
	const node = config.apiNode;
	const clientKeyPair = createKeyPairFromPrivateKeyString(config.clientPrivateKey);
	const aliveConnections = {};

	return {
		/**
		 * Leases an available connection.
		 * @returns {module:connection/catapultConnection~CatapultConnection} A connection.
		 */
		lease: () => {
			const connection = aliveConnections[node];
			if (connection)
				return Promise.resolve(connection);

			return new Promise((resolve, reject) => {
				logger(`connecting to ${node.host}:${node.port}`);
				const serverSocket = connectionFactory(node.port, node.host);

				const apiNodePublicKey = convert.hexToUint8(node.publicKey);

				serverSocket
						.on('error', () => {
							// capture error, otherwise net default handler will be called
							// default error handler issues reject(), that would go through bootstraper and toRestError().
							// the result might contain information about api node IP and port, because it might be different host,
							// that information shouldn't be available to rest clients.
						})
						.on('close', () => {
							delete aliveConnections[node];
							reject(errors.createServiceUnavailableError('connection failed'));
						});

				return authPromiseFactory(serverSocket, clientKeyPair, apiNodePublicKey, logger)
					.then(() => {
						// wrap the socket in a catapult connection and save it
						const serverConnection = catapultConnection.wrap(serverSocket);
						aliveConnections[node] = serverConnection;
						resolve(serverConnection);
					}, reject);
			});
		}
	};
}
