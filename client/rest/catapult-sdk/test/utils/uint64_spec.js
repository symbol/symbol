import { expect } from 'chai';
import uint64 from '../../src/utils/uint64';

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

	const hexTestCases = [
		{ str: '0000000000000000', value: [0, 0], description: '0' },
		{ str: '000000000000A1B2', value: [0xA1B2, 0], description: '(0, 8)' },
		{ str: '0000000012345678', value: [0x12345678, 0], description: '8' },
		{ str: '0000ABCD12345678', value: [0x12345678, 0xABCD], description: '(8, 16)' },
		{ str: '1234567890ABCDEF', value: [0x90ABCDEF, 0x12345678], description: '16' },
		{ str: 'FFFFFFFFFFFFFFFF', value: [0xFFFFFFFF, 0xFFFFFFFF], description: '16 (max value)' }
	];

	describe('fromHex', () => {
		for (const testCase of hexTestCases) {
			it(`can parse hex string with ${testCase.description} significant digits`, () => {
				// Act:
				const value = uint64.fromHex(testCase.str);

				// Assert:
				expect(value).to.deep.equal(testCase.value);
			});
		}

		it('cannot parse hex string with invalid characters into uint64', () => {
			// Assert:
			expect(() => { uint64.fromHex('0000000012345G78'); }).to.throw('unrecognized hex char'); // contains 'G'
		});

		it('cannot parse hex string with invalid size into uint64', () => {
			// Assert:
			expect(() => { uint64.fromHex(''); }).to.throw('hex string has unexpected size'); // empty string
			expect(() => { uint64.fromHex('1'); }).to.throw('hex string has unexpected size'); // odd number of chars
			expect(() => { uint64.fromHex('ABCDEF12'); }).to.throw('hex string has unexpected size'); // too short
			expect(() => { uint64.fromHex('1234567890ABCDEF12'); }).to.throw('hex string has unexpected size'); // too long
		});
	});

	describe('toHex', () => {
		for (const testCase of hexTestCases) {
			it(`can format hex string with ${testCase.description} significant digits`, () => {
				// Act:
				const str = uint64.toHex(testCase.value);

				// Assert:
				expect(str).to.equal(testCase.str);
			});
		}
	});

	describe('isZero', () => {
		const zeroTestCases = [
			{ description: 'low and high are zero', value: [0, 0], isZero: true },
			{ description: 'low is nonzero and high is zero', value: [1, 0], isZero: false },
			{ description: 'low is zero and high is nonzero', value: [0, 1], isZero: false },
			{ description: 'low and high are nonzero', value: [74, 12], isZero: false }
		];

		for (const testCase of zeroTestCases) {
			it(`returns ${testCase.isZero} when ${testCase.description}`, () => {
				// Act:
				const isZero = uint64.isZero(testCase.value);

				// Assert:
				expect(isZero).to.equal(testCase.isZero);
			});
		}
	});
});
