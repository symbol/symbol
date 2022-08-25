const { hexToUint8 } = require('../../src/utils/converter');
const { isCanonicalS } = require('../../src/utils/cryptoHelpers');
const { expect } = require('chai');

describe('cryptoHelpers', () => {
	describe('isCanonicalS', () => {
		it('returns true for canonical S', () => {
			// Act:
			const result = isCanonicalS(hexToUint8('FD98CAD07CEA0D72ED7BC476C697EA415062CDC0C753172D4C308A586586510E'));

			// Assert:
			expect(result).to.equal(true);
		});

		it('returns false for non-canonical S', () => {
			// Act:
			const result = isCanonicalS(hexToUint8('EA6CC02D974D20CAC318BC19A591C9565062CDC0C753172D4C308A586586511E'));

			// Assert:
			expect(result).to.equal(false);
		});
	});
});
