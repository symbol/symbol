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

/** @module auth/auth */
const PacketParser = require('../parser/PacketParser');
const verifyPeer = require('./verifyPeer');
const VerifyResult = require('./VerifyResult');

/**
 * An error that indicates a failed verification handshake.
 * @property {module:auth/VerifyResult} verifyResult The result of a verify operation.
 */
class VerifyError extends Error {
	/**
	 * Creates a new verify error.
	 * @param {module:auth/VerifyResult} result The result of a verify operation.
	 */
	constructor(result) {
		super(`verify failed with ${result}`);
		this.verifyResult = result;
	}
}

const auth = {
	/**
	 * Starts an authentication handshake with a catapult server.
	 * @param {net.Socket} serverSocket A socket connection to the catapult server.
	 * @param {module:crypto/keyPair~KeyPair} clientKeyPair The key pair of the connecting client.
	 * @param {module:crypto/keyPair~PublicKey} serverPublicKey The public key of the catapult server.
	 * @param {Function} [logger=empty] A logging function that is passed status messages.
	 * @returns {Promise}
	 * A promise that is completed when authentication completes and either resolved with a
	 * {@link module:parser/PacketParser} on success or rejected with a
	 * {@link module:auth/auth~VerifyError} on failure.
	 */
	createAuthPromise: (serverSocket, clientKeyPair, serverPublicKey, logger = () => {}) =>
		new Promise((resolve, reject) => {
			// create a verifier
			const verifier = verifyPeer.createServerVerifier(serverSocket, clientKeyPair, serverPublicKey);
			verifier.on('status', logger);

			// create a packet parser and forward packets to the verifier
			const parser = new PacketParser();
			parser.onPacket(verifier.handler);

			// hook server socket events
			serverSocket
				.on('data', data => {
					// send socket data to the parser
					logger(`received data with size ${data.length}`);
					parser.push(data);
				})
				.on('close', () => {
					// fail if the socket is closed
					reject(new VerifyError(VerifyResult.ioError));
				});

			verifier.on('verify', result => {
				// stop forwarding packets to the verifier
				parser.impl.emitter.removeListener('packet', verifier.handler);

				// on success, complete the promise with the parser
				if (VerifyResult.success === result) {
					resolve(parser);
					return;
				}

				// otherwise, fail and reject the promise
				const verifyError = new VerifyError(result);
				reject(verifyError);
				serverSocket.destroy(verifyError);
			});
		})
};

module.exports = auth;
