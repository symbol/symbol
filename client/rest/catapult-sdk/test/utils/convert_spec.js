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
const { expect } = require('chai');

describe('convert', () => {
	describe('toByte', () => {
		it('can convert all valid hex char combinations to byte', () => {
			// Arrange:
			const charToValueMappings = [];
			for (let code = '0'.charCodeAt(0); code <= '9'.charCodeAt(0); ++code)
				charToValueMappings.push([String.fromCharCode(code), code - '0'.charCodeAt(0)]);
			for (let code = 'a'.charCodeAt(0); code <= 'f'.charCodeAt(0); ++code)
				charToValueMappings.push([String.fromCharCode(code), code - 'a'.charCodeAt(0) + 10]);
			for (let code = 'A'.charCodeAt(0); code <= 'F'.charCodeAt(0); ++code)
				charToValueMappings.push([String.fromCharCode(code), code - 'A'.charCodeAt(0) + 10]);

			// Act:
			let numTests = 0;
			charToValueMappings.forEach(pair1 => {
				charToValueMappings.forEach(pair2 => {
					// Act:
					const byte = convert.toByte(pair1[0], pair2[0]);

					// Assert:
					const expected = (pair1[1] * 16) + pair2[1];
					expect(byte, `input: ${pair1[0]}${pair2[0]}`).to.equal(expected);
					++numTests;
				});
			});

			// Sanity:
			expect(numTests).to.equal(22 * 22);
		});

		it('cannot convert invalid hex chars to byte', () => {
			// Arrange:
			const pairs = [['G', '6'], ['7', 'g'], ['*', '8'], ['9', '!']];

			// Act:
			pairs.forEach(pair => {
				// Assert:
				const message = `input: ${pair[0]}${pair[0]}`;
				expect(() => { convert.toByte(pair[0], pair[1]); }, message).to.throw('unrecognized hex char');
			});
		});
	});

	describe('isHexString', () => {
		it('returns true for valid hex strings', () => {
			// Arrange:
			const inputs = [
				'',
				'026ee415fc15',
				'abcdef0123456789ABCDEF'
			];

			// Act:
			inputs.forEach(input => {
				const isHexString = convert.isHexString(input);

				// Assert:
				expect(isHexString, `input ${input}`).to.equal(true);
			});
		});

		it('returns false for invalid hex strings', () => {
			// Arrange:
			const inputs = [
				'abcdef012345G789ABCDEF', // invalid ('G') char
				'abcdef0123456789ABCDE' // invalid (odd) length
			];

			// Act:
			inputs.forEach(input => {
				const isHexString = convert.isHexString(input);

				// Assert:
				expect(isHexString, `input ${input}`).to.equal(false);
			});
		});
	});

	describe('hexToUint8', () => {
		it('can parse empty hex string into array', () => {
			// Act:
			const actual = convert.hexToUint8('');

			// Assert:
			const expected = Uint8Array.of();
			expect(actual).to.deep.equal(expected);
		});

		it('can parse valid hex string into array', () => {
			// Act:
			const actual = convert.hexToUint8('026ee415fc15');

			// Assert:
			const expected = Uint8Array.of(0x02, 0x6E, 0xE4, 0x15, 0xFC, 0x15);
			expect(actual).to.deep.equal(expected);
		});

		it('can parse valid hex string containing all valid hex characters into array', () => {
			// Act:
			const actual = convert.hexToUint8('abcdef0123456789ABCDEF');

			// Assert:
			const expected = Uint8Array.of(0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF);
			expect(actual).to.deep.equal(expected);
		});

		it('cannot parse hex string with invalid characters into array', () => {
			// Assert:
			expect(() => { convert.hexToUint8('abcdef012345G789ABCDEF'); }).to.throw('unrecognized hex char');
		});

		it('cannot parse hex string with invalid size into array', () => {
			// Assert:
			expect(() => { convert.hexToUint8('abcdef012345G789ABCDE'); }).to.throw('hex string has unexpected size');
		});
	});

	describe('uint8ToHex', () => {
		it('can format empty array into hex string', () => {
			// Act:
			const actual = convert.uint8ToHex(Uint8Array.of());

			// Assert:
			expect(actual).to.equal('');
		});

		it('can format single value array into hex string', () => {
			// Act:
			const actual = convert.uint8ToHex(Uint8Array.of(0xD2));

			// Assert:
			expect(actual).to.equal('D2');
		});

		it('can format multi value array into hex string', () => {
			// Act:
			const actual = convert.uint8ToHex(Uint8Array.of(0x02, 0x6E, 0xE4, 0x15, 0xFC, 0x15));

			// Assert:
			expect(actual).to.equal('026EE415FC15');
		});
	});

	describe('tryParseUint', () => {
		const addTryParseSuccessTest = (name, str, expectedValue) => {
			it(name, () => {
				// Act:
				const value = convert.tryParseUint(str);

				// Assert:
				expect(value).to.equal(expectedValue);
			});
		};

		addTryParseSuccessTest('can parse decimal string', '14952', 14952);
		addTryParseSuccessTest('can parse zero decimal string', '0', 0);
		addTryParseSuccessTest('can parse decimal string with all digits', '1234567890', 1234567890);
		addTryParseSuccessTest('can parse decimal string with zeros', '10002', 10002);
		addTryParseSuccessTest('can parse max safe integer decimal string', Number.MAX_SAFE_INTEGER.toString(), 9007199254740991);

		const addTryParseFailureTest = (name, str) => {
			it(name, () => {
				// Act:
				const value = convert.tryParseUint(str);

				// Assert:
				expect(value).to.equal(undefined);
			});
		};

		addTryParseFailureTest('cannot parse decimal string with left padding', ' 14952');
		addTryParseFailureTest('cannot parse decimal string with right padding', '14952 ');
		addTryParseFailureTest('cannot parse decimal string too large', '9007199254740992');
		addTryParseFailureTest('cannot parse zeros string', '00');
		addTryParseFailureTest('cannot parse octal string', '0123');
		addTryParseFailureTest('cannot parse hex string', '0x14A52');
		addTryParseFailureTest('cannot parse double string', '14.52');
		addTryParseFailureTest('cannot parse negative decimal string', '-14952');
		addTryParseFailureTest('cannot parse arbitrary string', 'catapult');
	});

	describe('uint8ToUint32', () => {
		it('uint8 array with zero length can be converted to uint32 array', () => {
			// Act:
			const actual = convert.uint8ToUint32(Uint8Array.of());

			// Assert:
			expect(actual).to.deep.equal(Uint32Array.of());
		});

		it('uint8 array with length multiple of four can be converted to uint32 array', () => {
			// Act:
			const actual = convert.uint8ToUint32(Uint8Array.of(0x02, 0x6E, 0x89, 0xAB, 0xCD, 0xEF, 0xE4, 0x15));

			// Assert:
			expect(actual).to.deep.equal(Uint32Array.of(0xAB896E02, 0x15E4EFCD));
		});

		it('uint8 array with length not multiple of four cannot be converted to uint32 array', () => {
			// Assert:
			expect(() => { convert.uint8ToUint32(Uint8Array.of(0x02, 0x6E, 0xE4, 0x15, 0x15)); })
				.to.throw('byte length of Uint32Array should be a multiple of 4');
		});
	});

	describe('uint32ToUint8', () => {
		it('uint32 array with zero length can be converted to uint8 array', () => {
			// Act:
			const actual = convert.uint32ToUint8(Uint32Array.of());

			// Assert:
			expect(actual).to.deep.equal(Uint8Array.of());
		});

		it('uint32 array with nonzero length can be converted to uint8 array', () => {
			// Act:
			const actual = convert.uint32ToUint8(Uint32Array.of(0xAB896E02, 0x15E4EFCD));

			// Assert:
			expect(actual).to.deep.equal(Uint8Array.of(0x02, 0x6E, 0x89, 0xAB, 0xCD, 0xEF, 0xE4, 0x15));
		});
	});

	describe('signed <-> unsigned byte', () => {
		const testCases = [
			{ signed: -128, unsigned: 0x80, description: 'min negative' },
			{ signed: -127, unsigned: 0x81, description: 'min negative plus one' },
			{ signed: -87, unsigned: 0xA9, description: 'negative' },
			{ signed: -1, unsigned: 0xFF, description: 'negative one' },
			{ signed: 0, unsigned: 0, description: 'zero' },
			{ signed: 1, unsigned: 0x01, description: 'positive one' },
			{ signed: 57, unsigned: 0x39, description: 'positive' },
			{ signed: 126, unsigned: 0x7E, description: 'max positive minus one' },
			{ signed: 127, unsigned: 0x7F, description: 'max positive' }
		];

		describe('uint8ToInt8', () => {
			const failureTestCases = [
				{ input: 256, description: 'one too large' },
				{ input: 1000, description: 'very large' }
			];

			failureTestCases.forEach(testCase => {
				it(`cannot convert number that is ${testCase.description}`, () => {
					// Assert:
					expect(() => convert.uint8ToInt8(testCase.input)).to.throw(`input '${testCase.input}' is out of range`);
				});
			});

			testCases.forEach(testCase => {
				it(`can convert ${testCase.description}`, () => {
					// Act:
					const value = convert.uint8ToInt8(testCase.unsigned);

					// Assert:
					expect(value).to.equal(testCase.signed);
				});
			});
		});

		describe('int8ToUint8', () => {
			const failureTestCases = [
				{ input: -1000, description: 'very small' },
				{ input: -129, description: 'one too small' },
				{ input: 128, description: 'one too large' },
				{ input: 1000, description: 'very large' }
			];

			failureTestCases.forEach(testCase => {
				it(`cannot convert number that is ${testCase.description}`, () => {
					// Assert:
					expect(() => convert.int8ToUint8(testCase.input)).to.throw(`input '${testCase.input}' is out of range`);
				});
			});

			testCases.forEach(testCase => {
				it(`can convert ${testCase.description}`, () => {
					// Act:
					const value = convert.int8ToUint8(testCase.signed);

					// Assert:
					expect(value).to.equal(testCase.unsigned);
				});
			});
		});
	});

	describe('signed <-> unsigned 16bits integer', () => {
		const testCases = [
			{ signed: -32768, unsigned: 0x8000, description: 'min negative' },
			{ signed: -32767, unsigned: 0x8001, description: 'min negative plus one' },
			{ signed: -287, unsigned: 0xFEE1, description: 'negative' },
			{ signed: -1, unsigned: 0xFFFF, description: 'negative one' },
			{ signed: 0, unsigned: 0, description: 'zero' },
			{ signed: 1, unsigned: 0x0001, description: 'positive one' },
			{ signed: 257, unsigned: 0x0101, description: 'positive' },
			{ signed: 32766, unsigned: 0x7FFE, description: 'max positive minus one' },
			{ signed: 32767, unsigned: 0x7FFF, description: 'max positive' }
		];

		describe('uint16ToInt16', () => {
			const failureTestCases = [
				{ input: 65536, description: 'one too large' },
				{ input: 100000, description: 'very large' }
			];

			failureTestCases.forEach(testCase => {
				it(`cannot convert number that is ${testCase.description}`, () => {
					// Assert:
					expect(() => convert.uint16ToInt16(testCase.input)).to.throw(`input '${testCase.input}' is out of range`);
				});
			});

			testCases.forEach(testCase => {
				it(`can convert ${testCase.description}`, () => {
					// Act:
					const value = convert.uint16ToInt16(testCase.unsigned);

					// Assert:
					expect(value).to.equal(testCase.signed);
				});
			});
		});

		describe('int16ToUint16', () => {
			const failureTestCases = [
				{ input: -100000, description: 'very small' },
				{ input: -32769, description: 'one too small' },
				{ input: 32768, description: 'one too large' },
				{ input: 100000, description: 'very large' }
			];

			failureTestCases.forEach(testCase => {
				it(`cannot convert number that is ${testCase.description}`, () => {
					// Assert:
					expect(() => convert.int16ToUint16(testCase.input)).to.throw(`input '${testCase.input}' is out of range`);
				});
			});

			testCases.forEach(testCase => {
				it(`can convert ${testCase.description}`, () => {
					// Act:
					const value = convert.int16ToUint16(testCase.signed);

					// Assert:
					expect(value).to.equal(testCase.unsigned);
				});
			});
		});
	});

	describe('signed <-> unsigned 32bits integer', () => {
		const testCases = [
			{ signed: -2147483648, unsigned: 0x80000000, description: 'min negative' },
			{ signed: -2147483647, unsigned: 0x80000001, description: 'min negative plus one' },
			{ signed: -2877657, unsigned: 0xFFD41727, description: 'negative' },
			{ signed: -1, unsigned: 0xFFFFFFFF, description: 'negative one' },
			{ signed: 0, unsigned: 0, description: 'zero' },
			{ signed: 1, unsigned: 0x01, description: 'positive one' },
			{ signed: 4565655, unsigned: 0x0045AA97, description: 'positive' },
			{ signed: 2147483646, unsigned: 0x7FFFFFFE, description: 'max positive minus one' },
			{ signed: 2147483647, unsigned: 0x7FFFFFFF, description: 'max positive' }
		];

		describe('uint32ToInt32', () => {
			const failureTestCases = [
				{ input: 4294967296, description: 'one too large' },
				{ input: 100000000000, description: 'very large' }
			];

			failureTestCases.forEach(testCase => {
				it(`cannot convert number that is ${testCase.description}`, () => {
					// Assert:
					expect(() => convert.uint32ToInt32(testCase.input)).to.throw(`input '${testCase.input}' is out of range`);
				});
			});

			testCases.forEach(testCase => {
				it(`can convert ${testCase.description}`, () => {
					// Act:
					const value = convert.uint32ToInt32(testCase.unsigned);

					// Assert:
					expect(value).to.equal(testCase.signed);
				});
			});
		});

		describe('int32ToUint32', () => {
			const failureTestCases = [
				{ input: -100000000000, description: 'very small' },
				{ input: -2147483649, description: 'one too small' },
				{ input: 4294967296, description: 'one too large' },
				{ input: 100000000000, description: 'very large' }
			];

			failureTestCases.forEach(testCase => {
				it(`cannot convert number that is ${testCase.description}`, () => {
					// Assert:
					expect(() => convert.int32ToUint32(testCase.input)).to.throw(`input '${testCase.input}' is out of range`);
				});
			});

			testCases.forEach(testCase => {
				it(`can convert ${testCase.description}`, () => {
					// Act:
					const value = convert.int32ToUint32(testCase.signed);

					// Assert:
					expect(value).to.equal(testCase.unsigned);
				});
			});
		});
	});
});
