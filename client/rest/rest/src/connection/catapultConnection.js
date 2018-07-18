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

const errors = require('../server/errors');
const catapult = require('catapult-sdk');

const { PacketParser } = catapult.parser;

const rejectOnClose = reject => () => reject(errors.createServiceUnavailableError('connection failed'));

/**
 * A catapult connection for interacting with api nodes.
 * @class CatapultConnection
 */
module.exports = {
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
				const innerReject = rejectOnClose(reject);
				connection.once('close', innerReject);
				connection.write(payload, () => {
					connection.removeListener('close', innerReject);
					resolve();
				});
			}),

		/**
		 * Sends a payload and waits for a response.
		 * @param {Buffer} payload The payload to be sent through the connection.
		 * @param {number} timeoutMs The timeout after which the promise is rejected because data is missing.
		 * @returns {Promise} The promise that is resolved upon completion of the read operation.
		 */
		pushPull(payload, timeoutMs) {
			const listen = () => new Promise((resolve, reject) => {
				const innerReject = rejectOnClose(reject);
				const packetParser = new PacketParser();
				connection.once('close', innerReject);
				connection.on('data', data => {
					packetParser.push(data);
				});

				packetParser.onPacket(packet => {
					connection.removeListener('close', innerReject);
					connection.destroy();
					resolve(packet);
				});
			});
			const promise = this.send(payload)
				.then(listen);

			const timeout = new Promise((resolve, reject) => {
				const id = setTimeout(() => {
					clearTimeout(id);
					connection.destroy();
					rejectOnClose(reject)();
				}, timeoutMs);
			});
			return Promise.race([
				promise,
				timeout
			]);
		}
	})
};
