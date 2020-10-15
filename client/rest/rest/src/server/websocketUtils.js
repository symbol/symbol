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

const catapult = require('catapult-sdk');
const winston = require('winston');
const crypto = require('crypto');

const { base32 } = catapult.utils;

module.exports = {
	/**
	 * Creates an aggregate subscriber composed of all websocket subscribers to a single topic.
	 * @param {string} topic Subscribed topic from which the data was received.
	 * @param {array<WebSocket>} subscribers Websocket subscribers.
	 * @param {function} formatter Formatter for formatting data before sending.
	 * @returns {function} Aggregate subscriber.
	 */
	createMultisender: (topic, subscribers, formatter) => ({
		/**
		 * Sends data to all subscribers.
		 * @param {object} data Unformatted data.
		 */
		send: data => {
			data.topic = topic;
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
