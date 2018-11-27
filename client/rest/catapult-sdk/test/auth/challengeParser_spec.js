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

const challengeParser = require('../../src/auth/challengeParser');
const { expect } = require('chai');

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
