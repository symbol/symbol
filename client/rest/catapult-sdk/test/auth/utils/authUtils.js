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

const crypto = require('crypto');
const PacketType = require('../../../src/packet/PacketType');
const test = require('../../testUtils');
const { sign } = require('../../../src/crypto/keyPair');

const Challenge_Size = 64;

const authUtils = {
	packets: {
		generateServerChallengeRequest: () => ({
			type: PacketType.serverChallenge,
			challenge: crypto.randomBytes(Challenge_Size)
		}),
		generateClientChallengeResponse: (request, keyPair) => ({
			type: PacketType.clientChallenge,
			signature: sign(keyPair, request.challenge)
		}),
		parseServerChallengeResponse: response => {
			if (!response || 169 !== response.length)
				throw Error('server challenge response is invalid');

			return {
				size: response.readInt32LE(0),
				type: response.readInt32LE(4),
				challenge: response.slice(8, 8 + Challenge_Size),
				signature: response.slice(72, 72 + test.constants.sizes.signature),
				publicKey: response.slice(136, 136 + test.constants.sizes.signer),
				securityMode: response.readUInt8(168)
			};
		},

		parseServerChallengeRequest: request => {
			if (!request || 72 !== request.length)
				throw Error('server challenge request is invalid');

			return {
				size: request.readInt32LE(0),
				type: request.readInt32LE(4),
				challenge: request.slice(8, 8 + Challenge_Size)
			};
		},
		generateServerChallengeResponse: (challenge, keyPair, securityMode) => ({
			type: PacketType.serverChallenge,
			challenge,
			signature: sign(keyPair, Buffer.concat([challenge, Buffer.of(securityMode)])),
			publicKey: keyPair.publicKey,
			securityMode
		}),
		parseClientChallengeResponse: response => {
			if (!response || 72 !== response.length)
				throw Error('client challenge response is invalid');

			return {
				size: response.readInt32LE(0),
				type: response.readInt32LE(4),
				signature: response.slice(8, 8 + test.constants.sizes.signature)
			};
		}
	}
};
Object.assign(authUtils, test);
authUtils.random.challenge = () => test.random.bytes(Challenge_Size);

module.exports = authUtils;
