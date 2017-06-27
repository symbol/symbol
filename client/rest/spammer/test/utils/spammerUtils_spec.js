import { expect } from 'chai';
import utils from '../../src/utils/spammerUtils';

describe('utils', () => {
	const bigNumbers = [0x0100000000, 0x0100000001, 0x123456789, Number.MAX_SAFE_INTEGER];
	const unsafeIntegers = [Number.MAX_SAFE_INTEGER + 1, Number.MAX_SAFE_INTEGER + 123456, 0xFFFFFFFFFFFFFFFF];

	describe('random', () => {
		it('respects limits', () => {
			// Arrange:
			const max = 1234;
			const values = [];

			// Act:
			for (let i = 0; 10 > i; ++i)
				values.push(utils.random(max));

			// Assert:
			for (const value of values)
				expect(value).to.be.within(0, max);
		});

		it('accepts max uint32', () => {
			// Arrange:
			const max = 0xFFFFFFFF;

			// Act:
			const value = utils.random(max);

			// Assert:
			expect(value).to.be.within(0, max);
		});

		it('throws if param does not fit into 32 bit', () => {
			// Assert:
			for (const value of bigNumbers)
				expect(() => { utils.random(value); }, `${value} is too large`).to.throw(`${value} does not fit into 32 bits`);
		});
	});

	describe('toUint64', () => {
		function assertArray(value, expectedArray) {
			// Act:
			const uint64 = utils.toUint64(value);

			// Assert:
			expect(uint64).to.deep.equal(expectedArray);
		}

		it('returns expected array', () => {
			// Assert:
			assertArray(0, [0x00000000, 0x00000000]);
			assertArray(1, [0x00000001, 0x00000000]);
			assertArray(0x100000000, [0x00000000, 0x00000001]);
			assertArray(Number.MAX_SAFE_INTEGER, [0xFFFFFFFF, 0x001FFFFF]);
		});

		it('throws if param does not fit into 53 bit', () => {
			// Assert:
			for (const value of unsafeIntegers)
				expect(() => { utils.toUint64(value); }, `${value} is too large`).to.throw(`${value} does not fit into 53 bits`);
		});
	});

	describe('uint32ToBytes', () => {
		function assertArray(value, expectedArray) {
			// Act:
			const array = utils.uint32ToBytes(value);

			// Assert:
			expect(array).to.deep.equal(expectedArray);
		}

		it('returns expected array', () => {
			// Assert:
			assertArray(0x00, Uint8Array.of([0x00, 0x00, 0x00, 0x00]));
			assertArray(0x01, Uint8Array.of([0x01, 0x00, 0x00, 0x00]));
			assertArray(0x12345678, Uint8Array.of([0x78, 0x56, 0x34, 0x12]));
			assertArray(0xFFFFFFFF, Uint8Array.of([0xFF, 0xFF, 0xFF, 0xFF]));
		});

		it('throws if param does not fit into 32 bit', () => {
			// Assert:
			for (const value of bigNumbers)
				expect(() => { utils.uint32ToBytes(value); }, `${value} is too large`).to.throw(`${value} does not fit into 32 bits`);
		});
	});
});
