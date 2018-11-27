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

const handler = require('../../src/auth/challenge');
const PacketType = require('../../src/packet/PacketType');
const test = require('./utils/authUtils');
const { expect } = require('chai');
const { verify } = require('../../src/crypto/keyPair');

describe('challenge', () => {
	// region verify server challenges

	describe('generate server challenge response', () => {
		const createServerChallengeResponse = (request, keyPair, securityMode) =>
			test.packets.parseServerChallengeResponse(handler.generateServerChallengeResponse(request, keyPair, securityMode));

		it('creates appropriate response', () => {
			// Arrange:
			const request = test.packets.generateServerChallengeRequest();
			const keyPair = test.random.keyPair();

			// Act:
			const response = createServerChallengeResponse(request, keyPair, 8);

			// Assert:
			expect(response.size).to.equal(169);
			expect(response.type).to.equal(PacketType.serverChallenge);

			expect(response.challenge).to.not.deep.equal(new Uint8Array(64)); // challenge is non-zero
			expect(response.challenge).to.not.deep.equal(request.challenge); // challenge is not the same as the request challenge

			expect(response.publicKey).to.deep.equal(Buffer.from(keyPair.publicKey));
			expect(response.securityMode).to.equal(8);

			// - the challenge and security mode are signed
			const signedBuffer = Buffer.concat([request.challenge, Buffer.of(8)], request.challenge.length + 1);
			const isVerified = verify(keyPair.publicKey, signedBuffer, response.signature);
			expect(isVerified).to.equal(true);
		});

		it('creates random challenge', () => {
			// Arrange:
			const Num_Challenges = 25;
			const challenges = new Set();

			const request = test.packets.generateServerChallengeRequest();
			const keyPair = test.random.keyPair();

			// Act: generate Num_Challenges
			for (let i = 0; i < Num_Challenges; ++i) {
				const response = createServerChallengeResponse(request, keyPair, 8);
				challenges.add(response.challenge);
			}

			// Assert: all challenges are unique
			expect(challenges.size).to.equal(Num_Challenges);
		});
	});

	describe('verify client challenge response', () => {
		const createClientChallengeRequest = keyPair => {
			const serverRequest = test.packets.generateServerChallengeRequest();
			return test.packets.parseServerChallengeResponse(handler.generateServerChallengeResponse(serverRequest, keyPair, 8));
		};

		const assertClientChallengeResponseVerification = (mutateResponse, expectedIsVerified) => {
			// Arrange:
			const keyPair = test.random.keyPair();
			const request = createClientChallengeRequest(keyPair);
			const response = test.packets.generateClientChallengeResponse(request, keyPair);
			mutateResponse(response);

			// Act:
			const isVerified = handler.verifyClientChallengeResponse(response, keyPair.publicKey, request.challenge);

			// Assert:
			expect(isVerified).to.equal(expectedIsVerified);
		};

		it('returns true for good response', () => {
			// Assert:
			assertClientChallengeResponseVerification(() => {}, true);
		});

		it('returns false for bad response', () => {
			// Assert: invalidate the signature
			assertClientChallengeResponseVerification(response => { response.signature[0] ^= 0xFF; }, false);
		});
	});

	// endregion

	// region verify client challenges

	describe('generate server challenge request', () => {
		const createServerChallengeRequest = () => test.packets.parseServerChallengeRequest(handler.generateServerChallengeRequest());

		it('creates appropriate request', () => {
			// Act:
			const request = createServerChallengeRequest();

			// Assert:
			expect(request.size).to.equal(72);
			expect(request.type).to.equal(PacketType.serverChallenge);
			expect(request.challenge).to.not.deep.equal(new Uint8Array(64)); // challenge is non-zero
		});

		it('creates random challenge', () => {
			// Arrange:
			const Num_Challenges = 25;
			const challenges = new Set();

			// Act: generate Num_Challenges
			for (let i = 0; i < Num_Challenges; ++i) {
				const request = createServerChallengeRequest();
				challenges.add(request.challenge);
			}

			// Assert: all challenges are unique
			expect(challenges.size).to.equal(Num_Challenges);
		});
	});

	describe('verify server challenge response', () => {
		const assertServerChallengeResponseVerification = (mutateResponse, expectedIsVerified) => {
			// Arrange:
			const challenge = test.random.challenge();
			const keyPair = test.random.keyPair();
			const response = test.packets.generateServerChallengeResponse(challenge, keyPair, 8);
			mutateResponse(response);

			// Act:
			const isVerified = handler.verifyServerChallengeResponse(response, challenge);

			// Assert:
			expect(isVerified).to.equal(expectedIsVerified);
		};

		it('returns true for good response', () => {
			// Assert:
			assertServerChallengeResponseVerification(() => {}, true);
		});

		it('returns false for bad response with corrupt signature', () => {
			// Assert: invalidate the signature
			assertServerChallengeResponseVerification(response => { response.signature[0] ^= 0xFF; }, false);
		});

		it('returns false for bad response with corrupt security mode', () => {
			// Assert: change the security mode
			assertServerChallengeResponseVerification(response => { response.securityMode += 2; }, false);
		});
	});

	describe('generate client challenge response', () => {
		it('creates appropriate response', () => {
			// Arrange:
			const request = test.packets.generateServerChallengeResponse(test.random.challenge(), test.random.keyPair(), 8);
			const keyPair = test.random.keyPair();

			// Act:
			const response = test.packets.parseClientChallengeResponse(handler.generateClientChallengeResponse(request, keyPair));

			// Assert:
			expect(response.size).to.equal(72);
			expect(response.type).to.equal(PacketType.clientChallenge);

			const isVerified = verify(keyPair.publicKey, request.challenge, response.signature);
			expect(isVerified).to.equal(true);
		});
	});

	// endregion
});
