const catapult = require('catapult-sdk');
const crypto = require('crypto');
const winston = require('winston');

const { base32 } = catapult.utils;


module.exports = {
	/**
	 * Creates an aggregate subscriber composed of all websocket subscribers to a single topic.
	 * @param {array<WebSocket>} subscribers Websocket subscribers.
	 * @param {function} formatter Formatter for formatting data before sending.
	 * @returns {function} Aggregate subscriber.
	 */
	createMultisender: (subscribers, formatter) => ({
		/**
		 * Sends data to all subscribers.
		 * @param {object} data Unformatted data.
		 */
		send: data => {
			const view = formatter(data);
			subscribers.forEach(client => {
				winston.debug(`websocket ${client.uid}: multisender.send sending data to client`);
				client.send(view, err => {
					if (err) {
						winston.error(`websocket ${client.uid}: error sending data to websocket`, err);
						client.close();
					}
				});
			});
		},

		/**
		 * Closes all subscribers.
		 * This function is intended to be invoked when there is a generic channel error.
		 */
		close: () => {
			subscribers.forEach(client => {
				client.close();
			});
		}
	}),

	/**
	 * Performs an initial handshake with a new websocket client and registers listeners.
	 * @param {WebSocket} client Websocket client.
	 * @param {function} messageHandler Custom message handler.
	 */
	handshake(client, messageHandler) {
		client.uid = base32.encode(crypto.randomBytes(20));

		const closeWithError = (message, err) => {
			winston.error(`websocket ${client.uid}: ${message}`, err);
			client.close();
		};

		client.on('error', err => {
			closeWithError('error from websocket', err);
		});

		client.on('message', messageJson => {
			const errorInformation = messageHandler(messageJson);
			if (errorInformation)
				closeWithError(...errorInformation);
		});

		client.send(`{"uid": "${client.uid}"}`, err => {
			if (err)
				closeWithError('error sending data to websocket', err);
		});
	}
};
