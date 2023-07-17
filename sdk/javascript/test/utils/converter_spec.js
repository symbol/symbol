import * as converter from '../../src/utils/converter.js';
import { expect } from 'chai';

describe('converter', () => {
	describe('toByte', () => {
		it('can convert all valid hex char combinations to byte', () => {
			// Arrange:
			const charToValueMappings = [];
			for (let code = '0'.charCodeAt(0); code <= '9'.charCodeAt(0); ++code)
				charToValueMappings.push({ char: String.fromCharCode(code), code: code - '0'.charCodeAt(0) });
			for (let code = 'a'.charCodeAt(0); code <= 'f'.charCodeAt(0); ++code)
				charToValueMappings.push({ char: String.fromCharCode(code), code: code - 'a'.charCodeAt(0) + 10 });
			for (let code = 'A'.charCodeAt(0); code <= 'F'.charCodeAt(0); ++code)
				charToValueMappings.push({ char: String.fromCharCode(code), code: code - 'A'.charCodeAt(0) + 10 });

			// Act:
			let numTests = 0;
			charToValueMappings.forEach(pair1 => {
				charToValueMappings.forEach(pair2 => {
					// Act:
					const byte = converter.toByte(pair1.char, pair2.char);

					// Assert:
					const expected = (pair1.code * 16) + pair2.code;
					expect(byte, `input: ${pair1.char}${pair2.char}`).to.equal(expected);
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
				expect(() => { converter.toByte(pair[0], pair[1]); }, message).to.throw('unrecognized hex char');
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
				const isHexString = converter.isHexString(input);

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
				const isHexString = converter.isHexString(input);

				// Assert:
				expect(isHexString, `input ${input}`).to.equal(false);
			});
		});
	});

	describe('hexToUint8', () => {
		it('can parse empty hex string into array', () => {
			// Act:
			const actual = converter.hexToUint8('');

			// Assert:
			const expected = Uint8Array.of();
			expect(actual).to.deep.equal(expected);
		});

		it('can parse valid hex string into array', () => {
			// Act:
			const actual = converter.hexToUint8('026ee415fc15');

			// Assert:
			const expected = Uint8Array.of(0x02, 0x6E, 0xE4, 0x15, 0xFC, 0x15);
			expect(actual).to.deep.equal(expected);
		});

		it('can parse valid hex string containing all valid hex characters into array', () => {
			// Act:
			const actual = converter.hexToUint8('abcdef0123456789ABCDEF');

			// Assert:
			const expected = Uint8Array.of(0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF);
			expect(actual).to.deep.equal(expected);
		});

		it('cannot parse hex string with invalid characters into array', () => {
			// Assert:
			expect(() => { converter.hexToUint8('abcdef012345G789ABCDEF'); }).to.throw('unrecognized hex char');
		});

		it('cannot parse hex string with invalid size into array', () => {
			// Assert:
			expect(() => { converter.hexToUint8('abcdef012345G789ABCDE'); }).to.throw('hex string has unexpected size');
		});
	});

	describe('uint8ToHex', () => {
		it('can format empty array into hex string', () => {
			// Act:
			const actual = converter.uint8ToHex(Uint8Array.of());

			// Assert:
			expect(actual).to.equal('');
		});

		it('can format single value array into hex string', () => {
			// Act:
			const actual = converter.uint8ToHex(Uint8Array.of(0xD2));

			// Assert:
			expect(actual).to.equal('D2');
		});

		it('can format multi value array into hex string', () => {
			// Act:
			const actual = converter.uint8ToHex(Uint8Array.of(0x02, 0x6E, 0xE4, 0x15, 0xFC, 0x15));

			// Assert:
			expect(actual).to.equal('026EE415FC15');
		});
	});

	describe('tryParseUint', () => {
		const addTryParseSuccessTest = (name, str, expectedValue) => {
			it(name, () => {
				// Act:
				const value = converter.tryParseUint(str);

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
				const value = converter.tryParseUint(str);

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

	const assertSigned = (asserter, isBigIntTest) => {
		if (!isBigIntTest) {
			it('8-bit value', () => {
				asserter([0x00], 1, 0);
				asserter([0x7F], 1, 127);
				asserter([0x80], 1, -128);
				asserter([0x81], 1, -127);
				asserter([0xFF], 1, -1);
			});

			it('16-bit value', () => {
				asserter([0x00, 0x00], 2, 0);
				asserter([0xFF, 0x7F], 2, 32767);
				asserter([0x00, 0x80], 2, -32768);
				asserter([0x01, 0x80], 2, -32767);
				asserter([0xFF, 0xFF], 2, -1);
			});

			it('32-bit value', () => {
				asserter([0x00, 0x00, 0x00, 0x00], 4, 0);
				asserter([0xFF, 0xFF, 0xFF, 0x7F], 4, 2147483647);
				asserter([0x00, 0x00, 0x00, 0x80], 4, -2147483648);
				asserter([0x01, 0x00, 0x00, 0x80], 4, -2147483647);
				asserter([0xFF, 0xFF, 0xFF, 0xFF], 4, -1);
			});
		} else {
			it('64-bit value', () => {
				asserter([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], 8, 0n);
				asserter([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F], 8, 9223372036854775807n);
				asserter([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80], 8, -9223372036854775808n);
				asserter([0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80], 8, -9223372036854775807n);
				asserter([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF], 8, -1n);
			});
		}
	};

	const assertUnsigned = (asserter, isBigIntTest) => {
		if (!isBigIntTest) {
			it('8-bit value', () => {
				asserter([0x00], 1, 0);
				asserter([0x7F], 1, 127);
				asserter([0x80], 1, 128);
				asserter([0x81], 1, 129);
				asserter([0xFF], 1, 255);
			});

			it('16-bit value', () => {
				asserter([0x00, 0x00], 2, 0);
				asserter([0xFF, 0x7F], 2, 32767);
				asserter([0x00, 0x80], 2, 32768);
				asserter([0x01, 0x80], 2, 32769);
				asserter([0xFF, 0xFF], 2, 65535);
			});

			it('32-bit value', () => {
				asserter([0x00, 0x00, 0x00, 0x00], 4, 0);
				asserter([0xFF, 0xFF, 0xFF, 0x7F], 4, 2147483647);
				asserter([0x00, 0x00, 0x00, 0x80], 4, 2147483648);
				asserter([0x01, 0x00, 0x00, 0x80], 4, 2147483649);
				asserter([0xFF, 0xFF, 0xFF, 0xFF], 4, 4294967295);
			});
		} else {
			it('64-bit value', () => {
				asserter([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], 8, BigInt(0n));
				asserter([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F], 8, BigInt(9223372036854775807n));
				asserter([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80], 8, BigInt(9223372036854775808n));
				asserter([0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80], 8, BigInt(9223372036854775809n));
				asserter([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF], 8, BigInt(18446744073709551615n));
			});
		}
	};

	const assertBytesToInt = (bytesToInt, isBigIntTest = false) => {
		const assertBytesToIntFactory = isSigned => (input, size, expectedValue) => {
			// Act:
			const actual = bytesToInt(new Uint8Array(input), size, isSigned);

			// Assert:
			expect(actual).to.equal(expectedValue);
		};

		describe('can convert signed', () => {
			assertSigned(assertBytesToIntFactory(true), isBigIntTest);
		});

		describe('can convert unsigned', () => {
			assertUnsigned(assertBytesToIntFactory(false), isBigIntTest);
		});
	};

	describe('bytesToInt', () => {
		const { bytesToInt } = converter;
		assertBytesToInt(bytesToInt);

		const assertBytesToIntOnMisaligned = (input, size, isSigned) => {
			// Arrange:
			const data = new Uint8Array(input);
			const misalignedBuffer = new Uint8Array(data.buffer, 1, size);

			// Act:
			expect(() => bytesToInt(misalignedBuffer, size, isSigned)).to.throw(/start offset of (Ui|I)nt.+Array should be/);
		};

		it('fails on misaligned access', () => {
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x80], 2, true);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x80], 2, false);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x80], 4, true);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x80], 4, false);
		});

		it('fails on unsupported integer size', () => {
			[3, 7, 8, 16].forEach(size => {
				expect(() => bytesToInt(new Uint8Array(32), size, true)).to.throw('unsupported int size');
				expect(() => bytesToInt(new Uint8Array(32), size, false)).to.throw('unsupported int size');
			});
		});
	});

	describe('bytesToBigInt', () => {
		const bytesToInt = converter.bytesToBigInt;
		assertBytesToInt(bytesToInt, true);

		const assertBytesToIntOnMisaligned = (input, size, isSigned) => {
			// Arrange:
			const data = new Uint8Array(input);
			const misalignedBuffer = new Uint8Array(data.buffer, 1, size);

			// Act:
			expect(() => bytesToInt(misalignedBuffer, size, isSigned)).to.throw(/start offset of Big(Ui|I)nt.+Array should be/);
		};

		it('fails on misaligned access', () => {
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x80], 8, true);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x80], 8, false);
		});

		it('fails on unsupported integer size', () => {
			[1, 2, 4, 16].forEach(size => {
				expect(() => bytesToInt(new Uint8Array(32), size, true)).to.throw('unsupported int size');
				expect(() => bytesToInt(new Uint8Array(32), size, false)).to.throw('unsupported int size');
			});
		});
	});

	describe('bytesToIntUnaligned', () => {
		const bytesToInt = converter.bytesToIntUnaligned;
		assertBytesToInt(bytesToInt);

		const assertBytesToIntOnMisaligned = (input, size, isSigned, expectedValue) => {
			// Arrange:
			const data = new Uint8Array(input);
			const misalignedBuffer = new Uint8Array(data.buffer, 1, size);

			// Act + Assert:
			expect(bytesToInt(misalignedBuffer, size, isSigned)).to.equal(expectedValue);
		};

		it('succeeds on misaligned access', () => {
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x80], 2, true, -32751);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x80], 2, false, 0x8011);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x80], 4, true, -2144132591);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x80], 4, false, 0x80332211);
		});

		it('fails on unsupported integer size', () => {
			[3, 7, 8, 16].forEach(size => {
				expect(() => bytesToInt(new Uint8Array(32), size, true)).to.throw('unsupported int size');
				expect(() => bytesToInt(new Uint8Array(32), size, false)).to.throw('unsupported int size');
			});
		});
	});

	describe('bytesToBigIntUnaligned', () => {
		const bytesToInt = converter.bytesToBigIntUnaligned;
		assertBytesToInt(bytesToInt, true);

		const assertBytesToIntOnMisaligned = (input, size, isSigned, expectedValue) => {
			// Arrange:
			const data = new Uint8Array(input);
			const misalignedBuffer = new Uint8Array(data.buffer, 1, size);

			// Act + Assert:
			expect(bytesToInt(misalignedBuffer, size, isSigned)).to.equal(expectedValue);
		};

		it('succeeds on misaligned access', () => {
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x80], 8, true, -9189763998223752687n);
			assertBytesToIntOnMisaligned([0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x80], 8, false, 0x8077665544332211n);
		});

		it('fails on unsupported integer size', () => {
			[1, 2, 4, 16].forEach(size => {
				expect(() => bytesToInt(new Uint8Array(32), size, true)).to.throw('unsupported int size');
				expect(() => bytesToInt(new Uint8Array(32), size, false)).to.throw('unsupported int size');
			});
		});
	});

	describe('intToBytes', () => {
		const assertIntToBytesFactory = isSigned => (expectedValues, size, value) => {
			// Act:
			const byteArray = converter.intToBytes(value, size, isSigned);

			// Assert:
			expect(byteArray).to.deep.equal(new Uint8Array(expectedValues));
		};

		describe('can convert signed', () => {
			assertSigned(assertIntToBytesFactory(true));
		});
		describe('can convert unsigned', () => {
			assertUnsigned(assertIntToBytesFactory(false));
		});
	});
});
