import BaseValue from '../src/BaseValue.js';
import { expect } from 'chai';

describe('BaseValue', () => {
	const canCreateBaseValueFactory = isSigned => (rawValue, size, expectedValue) => {
		// Act:
		const value = new BaseValue(size, rawValue, isSigned);

		// Assert:
		expect(value.size).to.equal(size);
		expect(value.value).to.equal(expectedValue);
	};

	it('cannot create 64-bit value with invalid type', () => {
		// Assert:
		expect(() => new BaseValue(8, -1)).to.throw('has invalid type');
		expect(() => new BaseValue(8, 1)).to.throw('has invalid type');
	});

	it('cannot create from dumb values', () => {
		// Assert:
		[1, 2, 4].forEach(byteSize => {
			expect(() => new BaseValue(byteSize, NaN)).to.throw('is not an integer');
			expect(() => new BaseValue(byteSize, -Infinity)).to.throw('is not an integer');
			expect(() => new BaseValue(byteSize, Infinity)).to.throw('is not an integer');
		});
	});

	it('cannot create unsigned with values outside range', () => {
		expect(() => new BaseValue(1, -1)).to.throw('outside of valid 8-bit range');
		expect(() => new BaseValue(1, 0x100)).to.throw('outside of valid 8-bit range');

		expect(() => new BaseValue(2, -1)).to.throw('outside of valid 16-bit range');
		expect(() => new BaseValue(2, 0x1_0000)).to.throw('outside of valid 16-bit range');

		expect(() => new BaseValue(4, -1)).to.throw('outside of valid 32-bit range');
		expect(() => new BaseValue(4, 0x1_0000_0000)).to.throw('outside of valid 32-bit range');

		expect(() => new BaseValue(8, -1n)).to.throw('outside of valid 64-bit range');
		expect(() => new BaseValue(8, 0x1_0000_0000_0000_0000n)).to.throw('outside of valid 64-bit range');
	});

	it('can create unsigned base value', () => {
		// Arrange:
		const canCreateUnsignedBaseValue = canCreateBaseValueFactory(false);

		// Assert:
		// - 8-bit
		[0, 0x24, 0xFF].forEach(rawValue => canCreateUnsignedBaseValue(rawValue, 1, rawValue));

		// - 16-bit
		[0, 0x243F, 0xFFFF].forEach(rawValue => canCreateUnsignedBaseValue(rawValue, 2, rawValue));

		// - 32-bit
		[0, 0x243F_6A88, 0xFFFF_FFFF].forEach(rawValue => canCreateUnsignedBaseValue(rawValue, 4, rawValue));

		// - 64-bit
		[0n, 0x243F_6A88_85A3_08D3n, 0xFFFF_FFFF_FFFF_FFFFn]
			.forEach(rawValue => canCreateUnsignedBaseValue(rawValue, 8, rawValue));
	});

	it('cannot create signed with values outised range', () => {
		expect(() => new BaseValue(1, -0x81, true)).to.throw('outside of valid 8-bit range');
		expect(() => new BaseValue(1, 0x80, true)).to.throw('outside of valid 8-bit range');

		expect(() => new BaseValue(2, -0x8001, true)).to.throw('outside of valid 16-bit range');
		expect(() => new BaseValue(2, 0x8000, true)).to.throw('outside of valid 16-bit range');

		expect(() => new BaseValue(4, -0x8000_0001, true)).to.throw('outside of valid 32-bit range');
		expect(() => new BaseValue(4, 0x8000_0000, true)).to.throw('outside of valid 32-bit range');

		expect(() => new BaseValue(8, -0x8000_0000_0000_0001n, true)).to.throw('outside of valid 64-bit range');
		expect(() => new BaseValue(8, 0x8000_0000_0000_0000n, true)).to.throw('outside of valid 64-bit range');
	});

	it('can create signed base value', () => {
		// Arrange:
		const canCreateSignedBaseValue = canCreateBaseValueFactory(true);

		// Assert:
		// - 8-bit
		[-0x80, 0x24, 0x7F].forEach(rawValue => canCreateSignedBaseValue(rawValue, 1, rawValue));

		// - 16-bit
		[-0x8000, 0x243F, 0x7FFF].forEach(rawValue => canCreateSignedBaseValue(rawValue, 2, rawValue));

		// - 32-bit
		[-0x8000_0000, 0x243F_6A88, 0x7FFF_FFFF].forEach(rawValue => canCreateSignedBaseValue(rawValue, 4, rawValue));

		// - 64-bit
		[-0x8000_0000_0000_0000n, 0x243F_6A88_85A3_08D3n, 0x7FFF_FFFF_FFFF_FFFFn]
			.forEach(rawValue => canCreateSignedBaseValue(rawValue, 8, rawValue));
	});

	const assertFormatting = (size, isSigned, testCases) => {
		testCases.forEach(testCase => {
			// Arrange:
			const value = new BaseValue(size, testCase[0], isSigned);

			// Act:
			const actual = value.toString();

			// Assert:
			expect(actual).to.equal(testCase[1]);
		});
	};

	it('toString of unsigned base values outputs fixed width hex', () => {
		assertFormatting(1, false, [[0, '0x00'], [0x24, '0x24'], [0xFF, '0xFF']]);
		assertFormatting(2, false, [[0, '0x0000'], [0x24, '0x0024'], [0x1234, '0x1234'], [0xFFFF, '0xFFFF']]);
		assertFormatting(4, false, [
			[0, '0x00000000'], [0x24, '0x00000024'], [0x1234, '0x00001234'], [0x12345678, '0x12345678'], [0xFFFFFFFF, '0xFFFFFFFF']
		]);
		assertFormatting(8, false, [
			[0n, '0x0000000000000000'], [0x24n, '0x0000000000000024'], [0x1234n, '0x0000000000001234'], [0x12345678n, '0x0000000012345678'],
			[0x1234567890ABCDEFn, '0x1234567890ABCDEF'], [0xFFFFFFFFFFFFFFFFn, '0xFFFFFFFFFFFFFFFF']
		]);
	});

	it('toString of signed base values outputs fixed width hex', () => {
		assertFormatting(1, true, [[0, '0x00'], [5, '0x05'], [127, '0x7F'], [-128, '0x80'], [-5, '0xFB'], [-1, '0xFF']]);
		assertFormatting(2, true, [
			[0, '0x0000'], [0x24, '0x0024'], [0x1234, '0x1234'], [0x7FFF, '0x7FFF'], [-0x8000, '0x8000'], [-5, '0xFFFB'], [-1, '0xFFFF']
		]);
		assertFormatting(4, true, [
			[0, '0x00000000'], [0x24, '0x00000024'], [0x1234, '0x00001234'], [0x12345678, '0x12345678'], [0x7FFFFFFF, '0x7FFFFFFF'],
			[-0x80000000, '0x80000000'], [-5, '0xFFFFFFFB'], [-1, '0xFFFFFFFF']
		]);
		assertFormatting(8, true, [
			[0n, '0x0000000000000000'], [0x24n, '0x0000000000000024'], [0x1234n, '0x0000000000001234'], [0x12345678n, '0x0000000012345678'],
			[0x1234567890ABCDEFn, '0x1234567890ABCDEF'], [0x7FFFFFFFFFFFFFFFn, '0x7FFFFFFFFFFFFFFF'],
			[-0x8000000000000000n, '0x8000000000000000'], [-5n, '0xFFFFFFFFFFFFFFFB'], [-1n, '0xFFFFFFFFFFFFFFFF']
		]);
	});
});
