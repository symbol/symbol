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

import catapult from '../../src/catapult-sdk/index.js';
import chainInfoCodec from '../../src/sockets/chainInfoCodec.js';
import { expect } from 'chai';

const { BinaryParser } = catapult.parser;

describe('chain info codec deserialize', () => {
	it('returns a deserialized object', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const packetBuffer = Buffer.concat([
			Buffer.from([0x89, 0xc2, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00]), // height
			Buffer.from([0x70, 0xc2, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00]), // finalizedHeight
			Buffer.from([0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]), // scoreHigh
			Buffer.from([0xd3, 0x45, 0x3a, 0x6f, 0x05, 0x24, 0x26, 0xaf]) // scoreLows
		]);
		binaryParser.push(Buffer.from(packetBuffer));

		// Act:
		const deserializedData = chainInfoCodec.deserialize(binaryParser);

		// Assert:
		expect(deserializedData).to.deep.equal({
			height: 3785353n,
			scoreLow: 12620814611511920083n,
			scoreHigh: 24n
		});
	});
});
