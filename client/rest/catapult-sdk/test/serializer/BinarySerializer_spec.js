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

const BinarySerializer = require('../../src/serializer/BinarySerializer');
const { expect } = require('chai');

describe('BinarySerializer', () => {
	// region constructor

	it('cannot construct binary serializer with non-positive integer size', () => {
		// Arrange:
		const message = 'BinarySerializer constructor needs integer size > 0';
		const createSerializer = param => new BinarySerializer(param);

		// Assert:
		expect(() => { createSerializer(0); }, 'zero size').to.throw(message);
		expect(() => { createSerializer(-1); }, 'negative size').to.throw(message);
		expect(() => { createSerializer(7.35); }, 'not an integer number').to.throw(message);
		expect(() => { createSerializer('foo'); }, 'not a number').to.throw(message);
	});

	it('can construct binary serializer from positive integer', () => {
		// Act:
		const serializer = new BinarySerializer(123);

		// Assert:
		expect(serializer.bufferSize()).to.equal(123);
	});

	// endregion

	// region writeUint8 / writeUint16 / writeUint32 / writeUint64 / writeBuffer

	const addTypeSerializerTests = (name, validData, dataLength, expected) => {
		it(`cannot serialize ${name} with insufficient bytes left in buffer`, () => {
			// Arrange: consume 1 byte to cause the insufficient buffer size problem
			const serializer = new BinarySerializer(dataLength);
			serializer.writeUint8(0);

			// Assert:
			const message = `insufficient buffer space left (${dataLength} bytes required, ${dataLength - 1} bytes available)`;
			expect(() => { serializer[name](validData); }).to.throw(message);
			expect(serializer.bufferSize()).to.equal(dataLength);
		});

		it(`can serialize ${name} with sufficient bytes left in buffer`, () => {
			// Arrange:
			const serializer = new BinarySerializer(dataLength);

			// Act:
			serializer[name](validData);

			// Assert:
			expect(serializer.bufferSize()).to.equal(dataLength);
			expect(serializer.buffer()).to.deep.equal(expected);
		});

		it(`can serialize ${name} with more than sufficient bytes left in buffer`, () => {
			// Arrange:
			const serializer = new BinarySerializer(dataLength + 2);

			// Act:
			serializer[name](validData);

			// Assert:
			const expectedBuffer = Buffer.concat([expected, Buffer.from([0x00, 0x00])], dataLength + 2);
			expect(serializer.bufferSize()).to.equal(dataLength + 2);
			expect(serializer.buffer()).to.deep.equal(expectedBuffer);
		});
	};

	addTypeSerializerTests('writeUint8', 0xDE, 1, Buffer.from([0xDE]));
	addTypeSerializerTests('writeUint16', 0xF393, 2, Buffer.from([0x93, 0xF3]));
	addTypeSerializerTests('writeUint32', 0x28D6A5F1, 4, Buffer.from([0xF1, 0xA5, 0xD6, 0x28]));
	addTypeSerializerTests('writeUint64', [0x38B0FE34, 0x7A01DB67], 8, Buffer.from([0x34, 0xFE, 0xB0, 0x38, 0x67, 0xDB, 0x01, 0x7A]));
	addTypeSerializerTests('writeBuffer', Buffer.from([0x1F, 0xEE, 0xC2, 0x34, 0x9D]), 5, Buffer.from([0x1F, 0xEE, 0xC2, 0x34, 0x9D]));

	// endregion

	// region multiple

	it('can serialize empty buffer', () => {
		// Arrange:
		const serializer = new BinarySerializer(5);

		// Act:
		serializer.writeUint8(0xF3);
		serializer.writeBuffer(Buffer.from([]));
		serializer.writeUint32(0xD18490FB);

		// Assert:
		expect(serializer.bufferSize()).to.equal(5);
		expect(serializer.buffer()).to.deep.equal(Buffer.from([0xF3, 0xFB, 0x90, 0x84, 0xD1]));
	});

	it('can serialize multiple entities', () => {
		// Arrange:
		const serializer = new BinarySerializer(10);

		// Act:
		serializer.writeUint8(0xF3);
		serializer.writeUint32(0xD18490FB);
		serializer.writeBuffer(Buffer.from([0xC8, 0x23, 0x6E, 0x5D, 0xA8]));

		// Assert:
		expect(serializer.bufferSize()).to.equal(10);
		expect(serializer.buffer()).to.deep.equal(Buffer.from([0xF3, 0xFB, 0x90, 0x84, 0xD1, 0xC8, 0x23, 0x6E, 0x5D, 0xA8]));
	});

	// endregion
});
