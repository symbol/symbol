const { BaseValue } = require('../src/BaseValue');
const { expect } = require('chai');

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
			expect(() => new BaseValue(byteSize, undefined)).to.throw('is not an integer');
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

	const assertFormatting = (size, rawValues, expected, isSigned) => {
		rawValues.forEach((rawValue, index) => {
			// Arrange
			const value = new BaseValue(size, rawValue, isSigned);

			// Act:
			const output = value.toString();

			// Assert:
			expect(output).to.equal(expected[index]);
		});
	};
	const assertUnsignedFormatting = (size, rawValues, expected) => {
		assertFormatting(size, rawValues, expected, false);
	};

	const assertSignedFormatting = (size, rawValues, expected) => {
		assertFormatting(size, rawValues, expected, true);
	};

	it('toString of unsigned base values outputs fixed width hex', () => {
		assertUnsignedFormatting(1, [0, 0x24, 0xFF], ['0x00', '0x24', '0xFF']);
		assertUnsignedFormatting(2, [0, 0x24, 0x1234, 0xFFFF], ['0x0000', '0x0024', '0x1234', '0xFFFF']);
		assertUnsignedFormatting(
			4,
			[0, 0x24, 0x1234, 0x12345678, 0xFFFFFFFF],
			['0x00000000', '0x00000024', '0x00001234', '0x12345678', '0xFFFFFFFF']
		);
		assertUnsignedFormatting(
			8,
			[0n, 0x24n, 0x1234n, 0x12345678n, 0x12345678_90ABCDEFn, 0xFFFFFFFF_FFFFFFFFn],
			[
				'0x0000000000000000',
				'0x0000000000000024',
				'0x0000000000001234',
				'0x0000000012345678',
				'0x1234567890ABCDEF',
				'0xFFFFFFFFFFFFFFFF'
			]
		);
	});

	it('toString of signed base values outputs fixed width hex', () => {
		assertSignedFormatting(1, [0, 5, 127, -128, -5, -1], ['0x00', '0x05', '0x7F', '0x80', '0xFB', '0xFF']);
		assertSignedFormatting(
			2,
			[0, 0x24, 0x1234, 0x7FFF, -0x8000, -5, -1],
			[
				'0x0000',
				'0x0024',
				'0x1234',
				'0x7FFF',
				'0x8000',
				'0xFFFB',
				'0xFFFF'
			]
		);
		assertSignedFormatting(
			4,
			[0, 0x24, 0x1234, 0x12345678, 0x7FFF_FFFF, -0x8000_0000, -5, -1],
			[
				'0x00000000',
				'0x00000024',
				'0x00001234',
				'0x12345678',
				'0x7FFFFFFF',
				'0x80000000',
				'0xFFFFFFFB',
				'0xFFFFFFFF'
			]
		);
		assertSignedFormatting(
			8,
			[0n, 0x24n, 0x1234n, 0x12345678n, 0x12345678_90ABCDEFn, 0x7FFFFFFF_FFFFFFFFn, -0x80000000_00000000n, -5n, -1n],
			[
				'0x0000000000000000',
				'0x0000000000000024',
				'0x0000000000001234',
				'0x0000000012345678',
				'0x1234567890ABCDEF',
				'0x7FFFFFFFFFFFFFFF',
				'0x8000000000000000',
				'0xFFFFFFFFFFFFFFFB',
				'0xFFFFFFFFFFFFFFFF'
			]
		);
	});
});
