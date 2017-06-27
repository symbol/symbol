/** @module auth/challengeParser */
import PacketType from '../packet/PacketType';

const headerInfos = {
	server: { type: PacketType.serverChallenge, size: 72 },
	client: { type: PacketType.clientChallenge, size: 72 }
};

function isPacketHeaderValid(packet, packetTypeName) {
	const headerInfo = headerInfos[packetTypeName];
	return packet.type === headerInfo.type && packet.size === headerInfo.size;
}

export default {
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
