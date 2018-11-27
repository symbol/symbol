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

/** @module auth/challenge */
const crypto = require('crypto');
const packetHeader = require('../packet/header');
const PacketType = require('../packet/PacketType');
const { sign, verify } = require('../crypto/keyPair');

const sizes = {
	header: packetHeader.size,
	challenge: 64,
	securityMode: 1
};

const challengeModule = {
	// region verify server challenges

	/**
	 * Generates a client response to a server challenge.
	 * @param {object} request The parsed server challenge request.
	 * @param {module:crypto/keyPair~KeyPair} keyPair The client key pair.
	 * @param {Numeric} securityMode The desired connection security mode.
	 * @returns {Buffer} A buffer composed of the binary response packet.
	 */
	generateServerChallengeResponse: (request, keyPair, securityMode) => {
		// create a new challenge
		const challenge = crypto.randomBytes(sizes.challenge);

		// sign the request challenge
		const signedBuffers = [request.challenge, Buffer.of(securityMode)];
		const signature = sign(keyPair, Buffer.concat(signedBuffers, sizes.challenge + sizes.securityMode));

		// create the response header
		const length = sizes.header + challenge.length + signature.length + keyPair.publicKey.length + sizes.securityMode;
		const header = packetHeader.createBuffer(PacketType.serverChallenge, length);

		// merge all buffers
		const buffers = [
			header,
			challenge,
			Buffer.from(signature.buffer),
			Buffer.from(keyPair.publicKey.buffer),
			Buffer.of(securityMode)
		];
		return Buffer.concat(buffers, length);
	},

	/**
	 * Verifies a server's response to a challenge.
	 * @param {object} response The parsed client challenge response.
	 * @param {module:crypto/keyPair~PublicKey} publicKey The server public key.
	 * @param {Uint8Array} challenge The challenge presented to the server.
	 * @returns {boolean} true if the response can be verified, false otherwise.
	 */
	verifyClientChallengeResponse: (response, publicKey, challenge) => verify(publicKey, challenge, response.signature),

	// endregion

	// region verify client challenges

	/**
	 * Generates a random server challenge that is sent to a client.
	 * @returns {Buffer} A buffer composed of the binary request packet.
	 */
	generateServerChallengeRequest: () => {
		// create a new challenge
		const challenge = crypto.randomBytes(sizes.challenge);

		// create the request header
		const length = sizes.header + sizes.challenge;
		const header = packetHeader.createBuffer(PacketType.serverChallenge, length);

		// merge all buffers
		const buffers = [header, challenge];
		return Buffer.concat(buffers, length);
	},

	/**
	 * Verifies a client's response to a challenge.
	 * @param {object} response The parsed server challenge response.
	 * @param {Uint8Array} challenge The challenge presented to the client.
	 * @returns {boolean} true if the response can be verified, false otherwise.
	 */
	verifyServerChallengeResponse: (response, challenge) => {
		const signedBuffers = [challenge, Buffer.of(response.securityMode)];
		return verify(response.publicKey, Buffer.concat(signedBuffers, sizes.challenge + sizes.securityMode), response.signature);
	},

	/**
	 * Generates a server response to a client challenge.
	 * @param {object} request The parsed client challenge request.
	 * @param {module:crypto/keyPair~KeyPair} keyPair The server key pair.
	 * @returns {Buffer} A buffer composed of the binary response packet.
	 */
	generateClientChallengeResponse: (request, keyPair) => {
		// sign the request challenge
		const signature = sign(keyPair, request.challenge);

		// create the response header
		const length = sizes.header + signature.length;
		const header = packetHeader.createBuffer(PacketType.clientChallenge, length);

		// merge all buffers
		const buffers = [header, Buffer.from(signature.buffer)];
		return Buffer.concat(buffers, length);
	}

	// endregion
};

module.exports = challengeModule;
