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

/** @module auth/challengeParser */
const PacketType = require('../packet/PacketType');

const headerInfos = {
	server: { type: PacketType.serverChallenge, size: 72 },
	client: { type: PacketType.clientChallenge, size: 72 }
};

const isPacketHeaderValid = (packet, packetTypeName) => {
	const headerInfo = headerInfos[packetTypeName];
	return packet.type === headerInfo.type && packet.size === headerInfo.size;
};

const challengeParser = {
	/**
	 * Tries to parse a server challenge request packet.
	 * @param {module:parser/PacketParser~RawPacket} packet The raw packet to parse.
	 * @returns {object} The parsed packet or undefined.
	 */
	tryParseServerChallengeRequest: packet => {
		if (!isPacketHeaderValid(packet, 'server'))
			return undefined;

		return {
			type: packet.type,
			challenge: packet.payload
		};
	},

	/**
	 * Tries to parse a client challenge request packet.
	 * @param {module:parser/PacketParser~RawPacket} packet The raw packet to parse.
	 * @returns {object} The parsed packet or undefined.
	 */
	tryParseClientChallengeResponse: packet => {
		if (!isPacketHeaderValid(packet, 'client'))
			return undefined;

		return {
			type: packet.type,
			signature: packet.payload
		};
	}
};

module.exports = challengeParser;
