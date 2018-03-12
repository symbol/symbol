const { expect } = require('chai');
const random = require('../../src/utils/random');

describe('random', () => {
	const bigNumbers = [0x0100000000, 0x0100000001, 0x123456789, Number.MAX_SAFE_INTEGER];

	describe('uint32', () => {
		it('respects limits', () => {
			// Arrange:
			const max = 1234;
			const values = [];

			// Act:
			for (let i = 0; 10 > i; ++i)
				values.push(random.uint32(max));

			// Assert:
			values.forEach(value => {
				expect(value).to.be.within(0, max);
			});
		});

		it('accepts max uint32', () => {
			// Arrange:
			const max = 0xFFFFFFFF;

			// Act:
			const value = random.uint32(max);

			// Assert:
			expect(value).to.be.within(0, max);
		});

		it('throws if param does not fit into 32 bit', () => {
			// Assert:
			bigNumbers.forEach(value => {
				expect(() => { random.uint32(value); }, `${value} is too large`).to.throw(`${value} does not fit into 32 bits`);
			});
		});
	});
});
