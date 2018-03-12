/** @module auth/verifyPeer */
const EventEmitter = require('events');
const challengeHandler = require('./challenge');
const challengeParser = require('./challengeParser');
const VerifyResult = require('./VerifyResult');

class AuthPacketHandler {
	constructor(serverSocket, clientKeyPair, serverPublicKey) {
		this.serverSocket = serverSocket;
		this.clientKeyPair = clientKeyPair;
		this.serverPublicKey = serverPublicKey;

		this.serverChallenge = undefined;
		this.hasRaisedVerifyEvent = false;
		this.emitter = new EventEmitter();

		// verify handshake requires successful processing of a server challenge and a client challenge
		this.authPacketHandlers = [
			packet => this.dispatch(packet, {
				name: 'server',
				handler: this.handleServerChallenge,
				tryParse: challengeParser.tryParseServerChallengeRequest
			}),
			packet => this.dispatch(packet, {
				name: 'client',
				handler: this.handleClientChallenge,
				tryParse: challengeParser.tryParseClientChallengeResponse
			})
		];
	}

	process(packet) {
		const handler = this.authPacketHandlers.shift();

		// if no handlers are left, the verify handshake is complete
		if (!handler)
			return;

		handler(packet);
	}

	handleServerChallenge(packet) {
		const response = challengeHandler.generateServerChallengeResponse(packet, this.clientKeyPair);
		this.serverChallenge = response.slice(8, 8 + 64);

		this.log(`writing response of length: ${response.length}`);
		this.serverSocket.write(response);
	}

	handleClientChallenge(packet) {
		const isVerified = challengeHandler.verifyClientChallengeResponse(packet, this.serverPublicKey, this.serverChallenge);
		this.log(`client challenge verified? ${isVerified}`);
		this.raiseVerified(isVerified ? VerifyResult.success : VerifyResult.failedChallenge);
	}

	dispatch(packet, traits) {
		const parsedPacket = traits.tryParse(packet);
		if (!parsedPacket) {
			this.log(`unable to parse ${traits.name} packet with type ${packet.type} and size ${packet.size}`);
			this.raiseVerified(VerifyResult.malformedData);
			return;
		}

		traits.handler.call(this, parsedPacket);
	}

	log(message) {
		this.emitter.emit('status', message);
	}

	raiseVerified(result) {
		// only raise the verify event once
		if (this.hasRaisedVerifyEvent)
			return;

		this.hasRaisedVerifyEvent = true;
		this.emitter.emit('verify', result);
	}
}

/**
 * Verifies a connection with a catapult server.
 * @class Verifier
 *
 * @fires status Messages about verification progress.
 * @fires verify The verification result.
 */

module.exports = {
	/**
	 * Creates a server verifier for performing a verification handshake with a catapult server.
	 * @param {net.Socket} serverSocket A socket connection to the catapult server.
	 * @param {module:crypto/keyPair~KeyPair} clientKeyPair The key pair of the connecting client.
	 * @param {module:crypto/keyPair~PublicKey} serverPublicKey The public key of the catapult server.
	 * @returns {module:auth/verifyPeer~Verifier} A verifier for the specified server.
	 */
	createServerVerifier(serverSocket, clientKeyPair, serverPublicKey) {
		const packetHandler = new AuthPacketHandler(serverSocket, clientKeyPair, serverPublicKey);
		const verifier = {
			/**
			 * Accepts a server payload.
			 * @param {Buffer} payload Data from the server.
			 * @memberof module:auth/verifyPeer~Verifier
			 * @instance
			 */
			handler: payload => { packetHandler.process(payload); },

			/**
			 * Subscribes to verifier events.
			 * @param {string} eventName The name of the event.
			 * @param {Function} eventHandler The function that should be called when the event is emitted.
			 * @returns {module:auth/verifyPeer~Verifier} The verifier (for chaining).
			 * @memberof module:auth/verifyPeer~Verifier
			 * @instance
			 */
			on: (eventName, eventHandler) => {
				packetHandler.emitter.on(eventName, eventHandler);
				return verifier;
			}
		};

		return verifier;
	}
};
