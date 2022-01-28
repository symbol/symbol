const { Reader } = require('../../src/utils/Reader');
const { expect } = require('chai');
const { describe } = require('yargs');

describe('Reader', () => {
	describe('constructor', () => {
		it('can create around uint8Array', () => {
			// Arrange:
			const byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6]);
			const reader = new Reader(byteArray);

			// Act:
			expect(reader.byteArray).to.deep.equal(new Uint8Array(byteArray.buffer));
			expect(reader.byteArray.buffer).to.equal(byteArray.buffer);
		});

		it('can create around view', () => {
			// Arrange:
			const byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6]);
			const view = new Uint8Array(byteArray.buffer, 2, 3);
			const reader = new Reader(view);

			// Act:
			expect(reader.byteArray).to.deep.equal(view);
			expect(reader.byteArray.buffer).to.equal(byteArray.buffer);
			expect(reader.byteArray.byteOffset).to.equal(view.byteOffset);
			expect(reader.byteArray.byteLength).to.equal(view.byteLength);
		});
	});

	describe('shrinked_buffer', () => {

	});
});
