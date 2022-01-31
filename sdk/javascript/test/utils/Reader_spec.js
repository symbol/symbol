const { Reader } = require('../../src/utils/Reader');
const { expect } = require('chai');

describe('Reader', () => {
	describe('constructor', () => {
		it('can create around whole buffer', () => {
			// Arrange:
			const byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6]);

			// Act:
			const reader = new Reader(byteArray);

			// Assert:
			expect(reader.byteArray).to.deep.equal(new Uint8Array([0, 1, 2, 3, 4, 5, 6]));
			expect(reader.byteArray.buffer).to.equal(byteArray.buffer);
		});

		it('can create around view', () => {
			// Arrange:
			const byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8]);
			const view = new Uint8Array(byteArray.buffer, 2, 5);

			// Act:
			const reader = new Reader(view);

			// Assert:
			expect(reader.byteArray).to.deep.equal(view);
			expect(reader.byteArray).to.deep.equal(new Uint8Array([2, 3, 4, 5, 6]));

			// - underlying buffer is the same
			expect(reader.byteArray.buffer).to.equal(byteArray.buffer);
			expect(reader.byteArray.byteOffset).to.equal(view.byteOffset);
			expect(reader.byteArray.byteLength).to.equal(view.byteLength);
		});
	});

	class TestContext {
		constructor() {
			this.byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]);
			this.view = new Uint8Array(this.byteArray.buffer, 2, 5);
			this.reader = new Reader(this.view);
		}
	}

	const assertModifyTest = (mutator, expected) => {
		// Arrange:
		const context = new TestContext();

		// Act:
		const result = mutator(context.reader);

		// Assert:
		expect(result).to.deep.equal(new Uint8Array(expected.buffer));

		// - underlying buffer is the same
		expect(result.buffer).to.equal(context.byteArray.buffer);
		expect(result.byteOffset).to.equal(context.view.byteOffset);
		expect(result.byteLength).to.equal(expected.size);
	};

	describe('shrinked_buffer', () => {
		const assertShrinkedBuffer = (newSize, expectedBuffer) => {
			assertModifyTest(
				reader => reader.shrinked_buffer(newSize),
				{ buffer: expectedBuffer, size: newSize }
			);
		};

		it('creates subview', () => {
			assertShrinkedBuffer(3, [2, 3, 4]);
		});

		it('can create non-shrinking subview', () => {
			assertShrinkedBuffer(5, [2, 3, 4, 5, 6]);
		});

		it('cannot create growing subview', () => {
			// Arrange:
			const context = new TestContext();

			// Assert:
			expect(() => context.reader.shrinked_buffer(6)).to.throw();
		});
	});

	describe('shrink', () => {
		const assertShrink = (newSize, expectedBuffer) => {
			assertModifyTest(
				reader => {
					reader.shrink(newSize);
					return reader.buffer;
				},
				{ buffer: expectedBuffer, size: newSize }
			);
		};

		it('creates subview', () => {
			assertShrink(3, [2, 3, 4]);
		});

		it('can create non-shrinking subview', () => {
			assertShrink(5, [2, 3, 4, 5, 6]);
		});

		it('cannot create growing subview', () => {
			// Arrange:
			const context = new TestContext();

			// Asert:
			expect(() => context.shrink(6)).to.throw();
		});
	});

	describe('shift', () => {
		const assertShift = (shiftCount, expectedBuffer) => {
			// Arrange:
			const context = new TestContext();

			// Act:
			context.reader.shift(shiftCount);
			const result = context.reader.buffer;

			// Assert:
			expect(result).to.deep.equal(new Uint8Array(expectedBuffer));

			// - underlying buffer is the same
			expect(result.buffer).to.equal(context.byteArray.buffer);
			expect(result.byteOffset).to.equal(context.view.byteOffset + shiftCount);
			expect(result.byteLength).to.equal(5 - shiftCount);
		};

		it('can shift subview by 0', () => {
			assertShift(0, [2, 3, 4, 5, 6]);
		});

		it('can shift subview by 1', () => {
			assertShift(1, [3, 4, 5, 6]);
		});

		it('can shift subview by more than 1', () => {
			assertShift(3, [5, 6]);
		});

		it('can shift by whole subview', () => {
			assertShift(5, []);
		});

		it('can do multiple shifts', () => {
			// Arrange:
			const context = new TestContext();

			// Act:
			context.reader.shift(2);
			context.reader.shift(2);
			const result = context.reader.buffer;

			// Assert:
			expect(result).to.deep.equal(new Uint8Array([6]));

			// - underlying buffer is the same
			expect(result.buffer).to.equal(context.byteArray.buffer);
			expect(result.byteOffset).to.equal(context.view.byteOffset + 4);
			expect(result.byteLength).to.equal(5 - 4);
		});

		it('cannot shift outside subview (single shift)', () => {
			// Arrange:
			const context = new TestContext();

			// Assert:
			expect(() => context.reader.shift(6)).to.throw();
		});

		it('cannot shift outside subview (multiple shifts)', () => {
			// Arrange:
			const context = new TestContext();
			context.reader.shift(4);

			// Sanity:
			expect(context.reader.buffer.byteOffset).to.equal(context.view.byteOffset + 4);

			// Assert:
			expect(() => context.reader.shift(2)).to.throw();
		});
	});
});
