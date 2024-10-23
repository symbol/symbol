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
import finalizedBlockCodec from '../../src/sockets/finalizedBlockCodec.js';
import { expect } from 'chai';

const { BinaryParser } = catapult.parser;

describe('finalized block codec deserialize', () => {
	it('returns a deserialized object', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const hash = Buffer.from([
			0x8e, 0x10, 0x37, 0xba, 0xeb, 0xb3, 0x4a, 0x7a, 0x2e, 0x6c, 0xb3, 0x90, 0xa5, 0xdf, 0xb5, 0xc3,
			0xf6, 0xe4, 0xba, 0xd4, 0xe6, 0x0c, 0x55, 0xa5, 0x4c, 0x8e, 0x8e, 0xb4, 0x8f, 0x01, 0xa0, 0xaf
		]);

		const packetBuffer = Buffer.concat([
			Buffer.from([0x46, 0x0a, 0x00, 0x00]), // finalizationEpoch
			Buffer.from([0x36, 0x00, 0x00, 0x00]), // finalizationPoint
			Buffer.from([0xc4, 0xc2, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00]), // height
			hash
		]);
		binaryParser.push(Buffer.from(packetBuffer));

		// Act:
		const deserializedData = finalizedBlockCodec.deserialize(binaryParser);

		// Assert:
		expect(deserializedData).to.deep.equal({
			finalizationEpoch: 2630,
			finalizationPoint: 54,
			height: 3785412n,
			hash
		});
	});
});
