const { hexToUint8 } = require('../../src/utils/converter');
const { ripemdKeccak256 } = require('../../src/utils/transforms');
const { expect } = require('chai');

describe('transforms', () => {
	it('can transform with ripemd keccak 256', () => {
		// Arrange:
		const payload = hexToUint8('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F');

		// Act:
		const hashResult = ripemdKeccak256(payload);

		// Assert:
		expect(hashResult).to.deep.equal(hexToUint8('FDB8D529F3656230A7FD6F183A0E8D750E4033C3'));
	});
});
