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

const SerializedSizeCalculator = require('../../src/serializer/SerializedSizeCalculator');
const { expect } = require('chai');

describe('EntitySizeCalculator', () => {
	const addTypeSerializerTests = (name, validData, expectedSize) => {
		it(`can serialize ${name}`, () => {
			// Arrange:
			const serializer = new SerializedSizeCalculator();

			// Act:
			serializer[name](validData);

			// Assert:
			expect(serializer.size()).to.equal(expectedSize);
		});
	};

	addTypeSerializerTests('writeUint8', 0xDE, 1);
	addTypeSerializerTests('writeUint16', 0xF393, 2);
	addTypeSerializerTests('writeUint32', 0x28D6A5F1, 4);
	addTypeSerializerTests('writeUint64', [0x38B0FE34, 0x7A01DB67], 8);
	addTypeSerializerTests('writeBuffer', Buffer.from([0x1F, 0xEE, 0xC2, 0x34, 0x9D]), 5);

	it('can serialize multiple entities', () => {
		// Act:
		const serializer = new SerializedSizeCalculator();

		// Act:
		serializer.writeUint8(0xF3);
		serializer.writeUint16(0x45D3);
		serializer.writeUint32(0xD18490FB);
		serializer.writeUint64([0xA34B06CE, 0xF974A0BC]);
		serializer.writeBuffer(Buffer.from([0xC8, 0x23, 0x6E, 0x5D, 0xA8]));

		// Assert:
		expect(serializer.size()).to.equal(1 + 2 + 4 + 8 + 5);
	});
});
