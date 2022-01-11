/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

const nodeTimeCodec = require('../../src/sockets/nodeTimeCodec');
const catapult = require('catapult-sdk');
const { expect } = require('chai');

const { BinaryParser } = catapult.parser;

describe('deserialize', () => {
	it('returns a deserialized node time object', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const packet = [0x90, 0xFA, 0x6D, 0x06, 0x01, 0x00, 0x00, 0x00, 0x90, 0xF8, 0x6D, 0x06, 0x10, 0x00, 0x00, 0x00];
		binaryParser.push(Buffer.from(packet));

		// Assert:
		expect(nodeTimeCodec.deserialize(binaryParser)).to.deep.equal({
			communicationTimestamps: {
				receiveTimestamp: [107870352, 16],
				sendTimestamp: [107870864, 1]
			}
		});
	});
});
