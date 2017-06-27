/** @module auth/challenge */
import crypto from 'crypto';
import { sign, verify } from '../crypto/keyPair';
import packetHeader from '../packet/header';
import PacketType from '../packet/PacketType';

const sizes = {
	header: packetHeader.size,
	challenge: 64
};

export default {
	// region verify server challenges

	/**
	 * Generates a client response to a server challenge.
	 * @param {object} request The parsed server challenge request.
	 * @param {module:crypto/keyPair~KeyPair} keyPair The client key pair.
	 * @returns {Buffer} A buffer composed of the binary response packet.
	 */
	generateServerChallengeResponse: (request, keyPair) => {
		// create a new challenge
		const challenge = crypto.randomBytes(sizes.challenge);

		// sign the request challenge
		const signature = sign(keyPair, request.challenge);

		// create the response header
		const length = sizes.header + challenge.length + signature.length + keyPair.publicKey.length;
		const header = packetHeader.createBuffer(PacketType.serverChallenge, length);

		// merge all buffers
		const buffers = [
			header,
			challenge,
			Buffer.from(signature.buffer),
			Buffer.from(keyPair.publicKey.buffer)
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
	verifyServerChallengeResponse: (response, challenge) => verify(response.publicKey, challenge, response.signature),

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
