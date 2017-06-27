import errors from '../server/errors';

/**
 * A catapult connection for interacting with api nodes.
 * @class CatapultConnection
 */
export default {
	/**
	 * Wraps a catapult connection around a socket connection.
	 * @param {net.Socket} connection The socket connection to wrap.
	 * @returns {object} A catapult connection wrapped around the socket connection.
	 */
	wrap: connection => ({
		/**
		 * Initiates a write operation.
		 * @param {Buffer} payload The payload to write.
		 * @returns {Promise} The promise that is resolved upon completion of the write operation.
		 */
		send: payload =>
			new Promise((resolve, reject) => {
				function rejectOnClose() {
					reject(errors.createServiceUnavailableError('connection failed'));
				}

				connection.once('close', rejectOnClose);

				connection.write(payload, () => {
					connection.removeListener('close', rejectOnClose);
					resolve();
				});
			})
	})
};
