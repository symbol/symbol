const { BaseValue } = require('../src/BaseValue');
const { expect } = require('chai');

describe('BaseValue', () => {
	const canCreateBaseValueFactory = isSigned => (value, size, expectedValue) => {
		// Act:
		const val = new BaseValue(size, value, isSigned);

		// Assert:
		expect(val.size).to.equal(size);
		expect(val.value).to.equal(expectedValue);
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
		[0, 0x24, 0xFF].forEach(value => canCreateUnsignedBaseValue(value, 1, value));

		// - 16-bit
		[0, 0x243F, 0xFFFF].forEach(value => canCreateUnsignedBaseValue(value, 2, value));

		// - 32-bit
		[0, 0x243F_6A88, 0xFFFF_FFFF].forEach(value => canCreateUnsignedBaseValue(value, 4, value));

		// - 64-bit
		[0n, 0x243F_6A88_85A3_08D3n, 0xFFFF_FFFF_FFFF_FFFFn]
			.forEach(value => canCreateUnsignedBaseValue(value, 8, value));
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
		[-0x80, 0x24, 0x7F].forEach(value => canCreateSignedBaseValue(value, 1, value));

		// - 16-bit
		[-0x8000, 0x243F, 0x7FFF].forEach(value => canCreateSignedBaseValue(value, 2, value));

		// - 32-bit
		[-0x8000_0000, 0x243F_6A88, 0x7FFF_FFFF].forEach(value => canCreateSignedBaseValue(value, 4, value));

		// - 64-bit
		[-0x8000_0000_0000_0000n, 0x243F_6A88_85A3_08D3n, 0x7FFF_FFFF_FFFF_FFFFn]
			.forEach(value => canCreateSignedBaseValue(value, 8, value));
	});
});
