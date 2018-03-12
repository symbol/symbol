const { expect } = require('chai');
const packetHeader = require('../../src/packet/header');

describe('packet header', () => {
	describe('constants', () => {
		it('has correct size', () => {
			// Act:
			const { size } = packetHeader;

			// Assert:
			expect(size).to.equal(8);
		});
	});

	describe('create buffer', () => {
		it('can create header buffer', () => {
			// Act:
			const buffer = packetHeader.createBuffer(0x1234, 0x987);

			// Assert:
			expect(buffer).to.deep.equal(Buffer.of(0x87, 0x09, 0x00, 0x00, 0x34, 0x12, 0x00, 0x00));
		});
	});
});
