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

const convert = require('../../src/utils/convert');
const uint64 = require('../../src/utils/uint64');
const { expect } = require('chai');

describe('uint64', () => {
	describe('compact', () => {
		it('can compact 32 bit value', () => {
			// Act:
			const result = uint64.compact([0x12345678, 0x00000000]);

			// Assert:
			expect(result).to.equal(0x12345678);
		});

		it('can compact less than max safe integer', () => {
			// Act:
			const result = uint64.compact([0x00ABCDEF, 0x000FDFFF]);

			// Assert:
			expect(result).to.equal(0xFDFFF00ABCDEF);
		});

		it('can compact max safe integer', () => {
			// Sanity:
			expect(0x1FFFFFFFFFFFFF).to.equal(Number.MAX_SAFE_INTEGER);

			// Act:
			const result = uint64.compact([0xFFFFFFFF, 0x001FFFFF]);

			// Assert:
			expect(result).to.equal(Number.MAX_SAFE_INTEGER);
		});

		it('cannot compact min unsafe integer', () => {
			// Sanity:
			expect(0x0020000000000000 + 1).to.equal(0x0020000000000000);

			// Act:
			const result = uint64.compact([0x00000000, 0x00200000]);

			// Assert:
			expect(result).to.deep.equal([0x00000000, 0x00200000]);
		});

		it('cannot compact greater than min unsafe integer', () => {
			// Act:
			const result = uint64.compact([0xF0000000, 0x01000D00]);

			// Assert:
			expect(result).to.deep.equal([0xF0000000, 0x01000D00]);
		});
	});

	describe('fromUint', () => {
		const failureTestCases = [
			{ number: 0x0020000000000000, description: 'min unsafe integer' },
			{ number: 0x01000D00F0000000, description: 'greater than min unsafe integer' },
			{ number: -1, description: 'negative' },
			{ number: 1234.56, description: 'floating point' }
		];

		failureTestCases.forEach(testCase => {
			it(`cannot parse number that is ${testCase.description}`, () => {
				// Assert:
				expect(() => uint64.fromUint(testCase.number)).to.throw(`number cannot be converted to uint '${testCase.number}'`);
			});
		});

		const successTestCases = [
			{ number: 0, uint64: [0, 0], description: '0' },
			{ number: 0xA1B2, uint64: [0xA1B2, 0], description: '(0, 8)' },
			{ number: 0x12345678, uint64: [0x12345678, 0], description: '8' },
			{ number: 0xABCD12345678, uint64: [0x12345678, 0xABCD], description: '(8, 16)' },
			{ number: 0x0014567890ABCDEF, uint64: [0x90ABCDEF, 0x00145678], description: '14' },
			{ number: Number.MAX_SAFE_INTEGER, uint64: [0xFFFFFFFF, 0x001FFFFF], description: '14 (max value)' }
		];

		successTestCases.forEach(testCase => {
			it(`can parse numeric with ${testCase.description} significant digits`, () => {
				// Act:
				const value = uint64.fromUint(testCase.number);

				// Assert:
				expect(value).to.deep.equal(testCase.uint64);
			});
		});
	});

	const hexTestCases = [
		{ str: '0000000000000000', value: [0, 0], description: '0' },
		{ str: '000000000000A1B2', value: [0xA1B2, 0], description: '(0, 8)' },
		{ str: '0000000012345678', value: [0x12345678, 0], description: '8' },
		{ str: '0000ABCD12345678', value: [0x12345678, 0xABCD], description: '(8, 16)' },
		{ str: '1234567890ABCDEF', value: [0x90ABCDEF, 0x12345678], description: '16' },
		{ str: 'FFFFFFFFFFFFFFFF', value: [0xFFFFFFFF, 0xFFFFFFFF], description: '16 (max value)' }
	];

	describe('fromBytes', () => {
		hexTestCases.forEach(testCase => {
			it(`can parse byte array with ${testCase.description} significant digits`, () => {
				// Arrange: prepare little-endian bytes
				const bytes = convert.hexToUint8(testCase.str).reverse();

				// Act:
				const value = uint64.fromBytes(bytes);

				// Assert:
				expect(value).to.deep.equal(testCase.value);
			});
		});

		it('cannot parse byte array with invalid size into uint64', () => {
			// Arrange:
			const errorMessage = 'byte array has unexpected size';

			// Assert:
			[0, 3, 4, 5, 7, 9].forEach(size => {
				expect(() => { uint64.fromBytes(new Uint8Array(size)); }, `size ${size}`).to.throw(errorMessage);
			});
		});
	});

	describe('toBytes', () => {
		it('can parse uint64 to Uint8Array', () => {
			const result = uint64.toBytes([0x00ABCDEF, 0x000FDFFF]);
			expect(result).to.deep.equal(new Uint8Array([0xEF, 0xCD, 0xAB, 0x00, 0xFF, 0xDF, 0x0F, 0x00]));
		});
	});

	describe('fromBytes32', () => {
		const fromBytes32TestCases = [
			{ str: '00000000', value: [0, 0], description: '0' },
			{ str: '0000A1B2', value: [0xA1B2, 0], description: '(0, 8)' },
			{ str: '12345678', value: [0x12345678, 0], description: '8' },
			{ str: 'FFFFFFFF', value: [0xFFFFFFFF, 0], description: '8 (max value)' }
		];

		fromBytes32TestCases.forEach(testCase => {
			it(`can parse byte array with ${testCase.description} significant digits`, () => {
				// Arrange: prepare little-endian bytes
				const bytes = convert.hexToUint8(testCase.str).reverse();

				// Act:
				const value = uint64.fromBytes32(bytes);

				// Assert:
				expect(value).to.deep.equal(testCase.value);
			});
		});

		it('cannot parse byte array with invalid size into uint64', () => {
			// Arrange:
			const errorMessage = 'byte array has unexpected size';

			// Assert:
			[0, 3, 5, 7, 8, 9].forEach(size => {
				expect(() => { uint64.fromBytes32(new Uint8Array(size)); }, `size ${size}`).to.throw(errorMessage);
			});
		});
	});

	describe('fromHex', () => {
		hexTestCases.forEach(testCase => {
			it(`can parse hex string with ${testCase.description} significant digits`, () => {
				// Act:
				const value = uint64.fromHex(testCase.str);

				// Assert:
				expect(value).to.deep.equal(testCase.value);
			});
		});

		it('cannot parse hex string with invalid characters into uint64', () => {
			// Assert:
			expect(() => { uint64.fromHex('0000000012345G78'); }).to.throw('unrecognized hex char'); // contains 'G'
		});

		it('cannot parse hex string with invalid size into uint64', () => {
			// Arrange:
			const errorMessage = 'hex string has unexpected size';

			// Assert:
			expect(() => { uint64.fromHex(''); }).to.throw(errorMessage); // empty string
			expect(() => { uint64.fromHex('1'); }).to.throw(errorMessage); // odd number of chars
			expect(() => { uint64.fromHex('ABCDEF12'); }).to.throw(errorMessage); // too short
			expect(() => { uint64.fromHex('1234567890ABCDEF12'); }).to.throw(errorMessage); // too long
		});
	});

	describe('toHex', () => {
		hexTestCases.forEach(testCase => {
			it(`can format hex string with ${testCase.description} significant digits`, () => {
				// Act:
				const str = uint64.toHex(testCase.value);

				// Assert:
				expect(str).to.equal(testCase.str);
			});
		});
	});

	describe('isZero', () => {
		const zeroTestCases = [
			{ description: 'low and high are zero', value: [0, 0], isZero: true },
			{ description: 'low is nonzero and high is zero', value: [1, 0], isZero: false },
			{ description: 'low is zero and high is nonzero', value: [0, 1], isZero: false },
			{ description: 'low and high are nonzero', value: [74, 12], isZero: false }
		];

		zeroTestCases.forEach(testCase => {
			it(`returns ${testCase.isZero} when ${testCase.description}`, () => {
				// Act:
				const isZero = uint64.isZero(testCase.value);

				// Assert:
				expect(isZero).to.equal(testCase.isZero);
			});
		});
	});

	describe('toString', () => {
		const successTestCases = [
			{ str: '0', value: [0, 0], description: 'min value' },
			{ str: '4294967295', value: [4294967295, 0], description: '8 significant digits' },
			{ str: '18446744069414584320', value: [0, 4294967295], description: '(0, 8) significant digits' },
			{ str: '193338964773', value: [65436453, 45], description: 'number' },
			{ str: '12774881867138931535', value: [3127188303, 2974383967], description: 'big number' },
			{ str: '18446744073709551615', value: [4294967295, 4294967295], description: 'max value' }
		];

		successTestCases.forEach(testCase => {
			it(`can parse uint64 values to string (${testCase.description})`, () => {
				expect(uint64.toString(testCase.value)).to.deep.equal(testCase.str);
			});
		});
	});

	describe('fromString', () => {
		const failureTestCases = [
			{ str: '', value: [4294967295, 4294967295], description: 'empty string' },
			{ str: undefined, value: [4294967295, 4294967295], description: 'undefined string' },
			{ str: null, value: [4294967295, 4294967295], description: 'null string' },
			{ str: '3546.5446', value: [4294967295, 4294967295], description: 'decimals' },
			{ str: '35,44,56\'46.5446', value: [4294967295, 4294967295], description: 'wrong characters' },
			{ str: 's4565678', value: [4294967295, 4294967295], description: 'wrong string' }
		];

		failureTestCases.forEach(testCase => {
			it(`cannot parse numeric strings into uint64 (${testCase.description})`, () => {
				expect(() => uint64.fromString(testCase.str)).to.throw(`input string is not a valid numeric string '${testCase.str}'`);
			});
		});

		const successTestCases = [
			{ str: '0', value: [0, 0], description: 'min value' },
			{ str: '5678', value: [5678, 0], description: 'small number' },
			{ str: '8765873000863846', value: [3663517798, 2040963], description: 'big number' },
			{ str: '4294967295', value: [4294967295, 0], description: 'max 32 bits' },
			{ str: '9007199254740993', value: [1, 2097152], description: 'max safe intger + 2' },
			{ str: '18446744073709551615', value: [4294967295, 4294967295], description: 'max value' }
		];

		successTestCases.forEach(testCase => {
			it(`can parse numeric strings into uint64 (${testCase.description})`, () => {
				expect(uint64.fromString(testCase.str)).to.deep.equal(testCase.value);
			});
		});
	});

	describe('multiply', () => {
		const successTestCases = [
			{
				factorA: [0, 0],
				factorB: [0, 0],
				result: [0, 0],
				description: 'min value'
			},
			{
				factorA: [25, 0],
				factorB: [4, 0],
				result: [100, 0],
				description: 'small value'
			},
			{
				factorA: [4294967295, 0],
				factorB: [4294967295, 0],
				result: [1, 4294967294],
				description: 'big value'
			},
			{
				factorA: [16843009, 16843009],
				factorB: [255, 0],
				result: [4294967295, 4294967295],
				description: 'max value'
			}
		];

		successTestCases.forEach(testCase => {
			it(`can multiply uint64 values (${testCase.description})`, () => {
				expect(uint64.multiply(testCase.factorA, testCase.factorB)).to.deep.equal(testCase.result);
			});
		});
	});
});
