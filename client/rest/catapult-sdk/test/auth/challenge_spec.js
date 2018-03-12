const { expect } = require('chai');
const { verify } = require('../../src/crypto/keyPair');
const PacketType = require('../../src/packet/PacketType');
const handler = require('../../src/auth/challenge');
const test = require('./utils/authUtils');

describe('challenge', () => {
	// region verify server challenges

	describe('generate server challenge response', () => {
		const createServerChallengeResponse = (request, keyPair) =>
			test.packets.parseServerChallengeResponse(handler.generateServerChallengeResponse(request, keyPair));

		it('creates appropriate response', () => {
			// Arrange:
			const request = test.packets.generateServerChallengeRequest();
			const keyPair = test.random.keyPair();

			// Act:
			const response = createServerChallengeResponse(request, keyPair);

			// Assert:
			expect(response.size).to.equal(168);
			expect(response.type).to.equal(PacketType.serverChallenge);

			expect(response.challenge).to.not.deep.equal(new Uint8Array(64)); // challenge is non-zero
			expect(response.challenge).to.not.deep.equal(request.challenge); // challenge is not the same as the request challenge

			const isVerified = verify(keyPair.publicKey, request.challenge, response.signature);
			expect(isVerified).to.equal(true);

			expect(response.publicKey).to.deep.equal(Buffer.from(keyPair.publicKey));
		});

		it('creates random challenge', () => {
			// Arrange:
			const Num_Challenges = 25;
			const challenges = new Set();

			const request = test.packets.generateServerChallengeRequest();
			const keyPair = test.random.keyPair();

			// Act: generate Num_Challenges
			for (let i = 0; i < Num_Challenges; ++i) {
				const response = createServerChallengeResponse(request, keyPair);
				challenges.add(response.challenge);
			}

			// Assert: all challenges are unique
			expect(challenges.size).to.equal(Num_Challenges);
		});
	});

	describe('verify client challenge response', () => {
		const createClientChallengeRequest = keyPair => {
			const serverRequest = test.packets.generateServerChallengeRequest();
			return test.packets.parseServerChallengeResponse(handler.generateServerChallengeResponse(serverRequest, keyPair));
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
			const response = test.packets.generateServerChallengeResponse(challenge, keyPair);
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

		it('returns false for bad response', () => {
			// Assert: invalidate the signature
			assertServerChallengeResponseVerification(response => { response.signature[0] ^= 0xFF; }, false);
		});
	});

	describe('generate client challenge response', () => {
		it('creates appropriate response', () => {
			// Arrange:
			const request = test.packets.generateServerChallengeResponse(test.random.challenge(), test.random.keyPair());
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
