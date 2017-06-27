import crypto from 'crypto';
import { sign } from '../../../src/crypto/keyPair';
import PacketType from '../../../src/packet/PacketType';
import test from '../../testUtils';

const Challenge_Size = 64;
const Signature_Size = 64;

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
			if (!response || 168 !== response.length)
				throw Error('server challenge response is invalid');

			return {
				size: response.readInt32LE(0),
				type: response.readInt32LE(4),
				challenge: response.slice(8, 8 + Challenge_Size),
				signature: response.slice(72, 72 + Signature_Size),
				publicKey: response.slice(136)
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
		generateServerChallengeResponse: (challenge, keyPair) => ({
			type: PacketType.serverChallenge,
			challenge,
			signature: sign(keyPair, challenge),
			publicKey: keyPair.publicKey
		}),
		parseClientChallengeResponse: response => {
			if (!response || 72 !== response.length)
				throw Error('client challenge response is invalid');

			return {
				size: response.readInt32LE(0),
				type: response.readInt32LE(4),
				signature: response.slice(8, 8 + Signature_Size)
			};
		}
	}
};
Object.assign(authUtils, test);
authUtils.random.challenge = () => test.random.bytes(Challenge_Size);

export default authUtils;
