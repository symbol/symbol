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

const BinaryParser = require('../../src/parser/BinaryParser');
const { expect } = require('chai');

describe('BinaryParser', () => {
	// region push

	it('cannot push non buffer', () => {
		// Arrange:
		const parser = new BinaryParser();

		// Act:
		const errorMessageText = 'NodeJS Buffer object';
		expect(() => { parser.push([0, 1, 2, 3]); }, 'array').to.throw(errorMessageText);
		expect(() => { parser.push({}); }, 'object').to.throw(errorMessageText);
		expect(() => { parser.push(7); }, 'numeric').to.throw(errorMessageText);

		// Assert:
		expect(parser.numUnprocessedBytes()).to.equal(0);
	});

	it('can push single buffer to parser', () => {
		// Arrange:
		const parser = new BinaryParser();

		// Act:
		parser.push(Buffer.alloc(3));

		// Assert:
		expect(parser.numUnprocessedBytes()).to.equal(3);
	});

	it('can push mutiple buffers to parser', () => {
		// Arrange:
		const parser = new BinaryParser();

		// Act:
		const numBuffers = 5;
		for (let i = 0; i < numBuffers; ++i)
			parser.push(Buffer.alloc(i + 1));

		// Assert:
		expect(parser.numUnprocessedBytes()).to.equal(15);
	});

	// endregion

	// region uint8 / uint16 / uint32 / uint64 / buffer

	const addTypeParserTests = (name, validData, expected) => {
		it(`cannot extract ${name} with insufficient data`, () => {
			// Arrange:
			const parser = new BinaryParser();
			parser.push(Buffer.alloc(validData.length - 1, 1));

			// Act:
			expect(() => { parser[name](validData.length); }).to.throw('insufficient unprocessed data');

			// Assert:
			expect(parser.numUnprocessedBytes()).to.equal(validData.length - 1);
		});

		it(`can extract ${name} with sufficient data`, () => {
			// Arrange:
			const parser = new BinaryParser();
			parser.push(Buffer.from(validData));

			// Act:
			const result = parser[name](validData.length);

			// Assert:
			expect(result).to.deep.equal(expected);
			expect(parser.numUnprocessedBytes()).to.equal(0);
		});

		it(`can extract ${name} with more than sufficient data`, () => {
			// Arrange:
			const parser = new BinaryParser();
			parser.push(Buffer.from(validData));
			parser.push(Buffer.from([0xAA, 0xBB]));

			// Act:
			const result = parser[name](validData.length);

			// Assert:
			expect(result).to.deep.equal(expected);
			expect(parser.numUnprocessedBytes()).to.equal(2);
		});

		it(`can extract ${name} with sufficient data spanning buffers`, () => {
			// Arrange:
			const parser = new BinaryParser();
			parser.push(Buffer.from([]));
			parser.push(Buffer.from(validData.slice(0, validData.length / 2)));
			parser.push(Buffer.from([]));
			parser.push(Buffer.from(validData.slice(validData.length / 2)));

			// Act:
			const result = parser[name](validData.length);

			// Assert:
			expect(result).to.deep.equal(expected);
			expect(parser.numUnprocessedBytes()).to.equal(0);
		});
	};

	addTypeParserTests('uint8', [0xFC], 0xFC);
	addTypeParserTests('uint16', [0x11, 0xC2], 0xC211);
	addTypeParserTests('uint32', [0x11, 0x22, 0x33, 0xAA], 0xAA332211);
	addTypeParserTests('uint64', [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88], [0x44332211, 0x88776655]);
	addTypeParserTests('buffer', [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77], Buffer.from([0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77]));

	// endregion

	it('can extract subset of buffer as buffer', () => {
		// Arrange:
		const parser = new BinaryParser();
		parser.push(Buffer.from([0x66, 0x22, 0x88, 0x99, 0xAA, 0xFF]));

		// Act:
		const result1 = parser.buffer(2);
		const result2 = parser.buffer(3);

		// Assert:
		expect(result1).to.deep.equal(Buffer.from([0x66, 0x22]));
		expect(result2).to.deep.equal(Buffer.from([0x88, 0x99, 0xAA]));
		expect(parser.numUnprocessedBytes()).to.equal(1);
	});

	it('can extract empty buffer as buffer', () => {
		// Arrange:
		const parser = new BinaryParser();
		parser.push(Buffer.from([0x66, 0x22, 0x88, 0x99, 0xAA, 0xFF]));

		// Act:
		const result1 = parser.buffer(2);
		const result2 = parser.buffer(0);

		// Assert:
		expect(result1).to.deep.equal(Buffer.from([0x66, 0x22]));
		expect(result2).to.deep.equal(Buffer.from([]));
		expect(parser.numUnprocessedBytes()).to.equal(4);
	});

	it('can extract multiple properties from parser', () => {
		// Arrange:
		const parser = new BinaryParser();
		parser.push(Buffer.from([0x11, 0x77, 0x33]));
		parser.push(Buffer.from([0x44]));
		parser.push(Buffer.from([0x55]));
		parser.push(Buffer.from([0x66, 0x22, 0x88, 0x99, 0xAA]));
		parser.push(Buffer.from([0xBB]));

		// Act:
		const result1 = parser.uint8();
		const result2 = parser.uint32();
		const result3 = parser.buffer(3);
		const result4 = parser.uint16();

		// Assert:
		expect(result1).to.equal(0x11);
		expect(result2).to.equal(0x55443377);
		expect(result3).to.deep.equal(Buffer.from([0x66, 0x22, 0x88]));
		expect(result4).to.equal(0xAA99);
		expect(parser.numUnprocessedBytes()).to.equal(1);
	});
});
