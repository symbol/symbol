const { expect } = require('chai');
const challengeParser = require('../../src/auth/challengeParser');

describe('challenge parser', () => {
	const Test_Buffer_64 = Buffer.of(
		0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD,
		0xEE, 0xEE, 0xFF, 0xFF, 0xAA, 0xAA, 0xBB, 0xBB,
		0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF,
		0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33,
		0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD,
		0xEE, 0xEE, 0xFF, 0xFF, 0xAA, 0xAA, 0xBB, 0xBB,
		0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF,
		0x12, 0x34, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33
	);

	const addParseTests = traits => {
		it('can parse from packet', () => {
			// Arrange:
			const packet = traits.createTemplatePacket();

			// Act:
			const parsedPacket = traits.tryParsePacket(packet);

			// Assert:
			expect(parsedPacket).to.deep.equal(traits.parsedTemplatePacket);
		});

		it('parse fails if size is incorrect', () => {
			// Arrange:
			const packet = traits.createTemplatePacket();
			++packet.size;

			// Act:
			const parsedPacket = traits.tryParsePacket(packet);

			// Assert:
			expect(parsedPacket).to.equal(undefined);
		});

		it('parse fails if type is incorrect', () => {
			// Arrange:
			const packet = traits.createTemplatePacket();
			++packet.type;

			// Act:
			const parsedPacket = traits.tryParsePacket(packet);

			// Assert:
			expect(parsedPacket).to.equal(undefined);
		});
	};

	describe('server challenge request', () => {
		addParseTests({
			createTemplatePacket: () => ({
				size: 0x00000048,
				type: 0x00000001,
				payload: Test_Buffer_64
			}),
			tryParsePacket: challengeParser.tryParseServerChallengeRequest,
			parsedTemplatePacket: { type: 0x0001, challenge: Test_Buffer_64 }
		});
	});

	describe('client challenge response', () => {
		addParseTests({
			createTemplatePacket: () => ({
				size: 0x00000048,
				type: 0x00000002,
				payload: Test_Buffer_64
			}),
			tryParsePacket: challengeParser.tryParseClientChallengeResponse,
			parsedTemplatePacket: { type: 0x0002, signature: Test_Buffer_64 }
		});
	});
});
