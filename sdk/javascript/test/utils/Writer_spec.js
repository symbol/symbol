const { Writer } = require('../../src/utils/Writer');
const { expect } = require('chai');

describe('Writer', () => {
	describe('can consturct', () => {
		it('with empty buffer', () => {
			// Arrange:
			const writer = new Writer(0);

			// Act:
			expect(() => writer.write([42])).to.throw();
		});

		it('with non-empty buffer', () => {
			// Arrange:
			const writer = new Writer(10);

			// Act:
			writer.write([42, 3, 14, 15]);

			// Assert:
			expect(writer.offset).to.equal(4);
			expect(writer.storage).to.deep.equal(new Uint8Array([42, 3, 14, 15, 0, 0, 0, 0, 0, 0]));
		});
	});

	describe('write', () => {
		it('all writes are saved', () => {
			// Arrange:
			const writer = new Writer(10);

			// Act:
			writer.write([42, 3, 14]);
			writer.write([15, 92, 65, 35]);

			// Assert:
			expect(writer.offset).to.equal(7);
			expect(writer.storage).to.deep.equal(new Uint8Array([42, 3, 14, 15, 92, 65, 35, 0, 0, 0]));
		});

		it('can write until the end of the buffer', () => {
			// Arrange:
			const writer = new Writer(10);

			// Act:
			writer.write([42, 3, 14, 15, 92, 65, 35]);
			writer.write([89, 79, 32]);

			// Assert:
			expect(writer.offset).to.equal(10);
			expect(writer.storage).to.deep.equal(new Uint8Array([42, 3, 14, 15, 92, 65, 35, 89, 79, 32]));
		});

		const assertCannotWritePastTheEnd = size => {
			// Arrange:
			const writer = new Writer(10);
			writer.write(new Uint8Array(size));

			// Sanity:
			expect(writer.offset).to.equal(size);

			// Assert:
			expect(() => writer.write([89, 79, 32, 38])).to.throw();
		};

		it('cannot write past the end of the buffer', () => {
			assertCannotWritePastTheEnd(7, 4);
			assertCannotWritePastTheEnd(10, 1);
		});
	});
});
