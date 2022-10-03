import BufferView from '../../src/utils/BufferView.js';
import { expect } from 'chai';

describe('BufferView', () => {
	// region constructor

	describe('constructor', () => {
		it('can create around whole buffer', () => {
			// Arrange:
			const byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6]);

			// Act:
			const view = new BufferView(byteArray);

			// Assert:
			expect(view.buffer).to.deep.equal(new Uint8Array([0, 1, 2, 3, 4, 5, 6]));
			expect(view.buffer.buffer).to.equal(byteArray.buffer);
		});

		it('can create around view', () => {
			// Arrange:
			const byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8]);
			const subView = new Uint8Array(byteArray.buffer, 2, 5);

			// Act:
			const view = new BufferView(subView);

			// Assert:
			expect(view.buffer).to.deep.equal(subView);
			expect(view.buffer).to.deep.equal(new Uint8Array([2, 3, 4, 5, 6]));

			// - underlying buffer is the same
			expect(view.buffer.buffer).to.equal(byteArray.buffer);
			expect(view.buffer.byteOffset).to.equal(subView.byteOffset);
			expect(view.buffer.byteLength).to.equal(subView.byteLength);
		});
	});

	// endregion

	// region shiftRight

	class TestContext {
		constructor() {
			this.byteArray = new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]);
			this.subView = new Uint8Array(this.byteArray.buffer, 2, 5);
			this.view = new BufferView(this.subView);
		}
	}

	describe('shiftRight', () => {
		const assertShift = (shiftCount, expectedBuffer) => {
			// Arrange:
			const context = new TestContext();

			// Act:
			context.view.shiftRight(shiftCount);
			const result = context.view.buffer;

			// Assert:
			expect(result).to.deep.equal(new Uint8Array(expectedBuffer));

			// - underlying buffer is the same
			expect(result.buffer).to.equal(context.byteArray.buffer);
			expect(result.byteOffset).to.equal(context.subView.byteOffset + shiftCount);
			expect(result.byteLength).to.equal(context.subView.byteLength - shiftCount);
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
			context.view.shiftRight(2);
			context.view.shiftRight(2);
			const result = context.view.buffer;

			// Assert:
			expect(result).to.deep.equal(new Uint8Array([6]));

			// - underlying buffer is the same
			expect(result.buffer).to.equal(context.byteArray.buffer);
			expect(result.byteOffset).to.equal(context.subView.byteOffset + 4);
			expect(result.byteLength).to.equal(5 - 4);
		});

		it('cannot shift outside subview (single shift)', () => {
			// Arrange:
			const context = new TestContext();

			// Act + Assert:
			expect(() => context.view.shiftRight(6)).to.throw('Invalid typed array length');
		});

		it('cannot shift outside subview (multiple shifts)', () => {
			// Arrange:
			const context = new TestContext();
			context.view.shiftRight(4);

			// Sanity:
			expect(context.view.buffer.byteOffset).to.equal(context.subView.byteOffset + 4);

			// Act + Assert:
			expect(() => context.view.shiftRight(2)).to.throw('Invalid typed array length');
		});
	});

	// endregion

	// region window + shrink

	const assertModifyTest = (mutator, expected) => {
		// Arrange:
		const context = new TestContext();

		// Act:
		const result = mutator(context.view);

		// Assert:
		expect(result).to.deep.equal(new Uint8Array(expected.buffer));

		// - underlying buffer is the same
		expect(result.buffer).to.equal(context.byteArray.buffer);
		expect(result.byteOffset).to.equal(context.subView.byteOffset);
		expect(result.byteLength).to.equal(expected.size);
	};

	describe('window', () => {
		const assertShrinkedBuffer = (newSize, expectedBuffer) => {
			assertModifyTest(
				view => view.window(newSize),
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

			// Act + Assert:
			expect(() => context.view.window(6)).to.throw('invalid shrink value');
		});
	});

	describe('shrink', () => {
		const assertShrink = (newSize, expectedBuffer) => {
			assertModifyTest(
				view => {
					view.shrink(newSize);
					return view.buffer;
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

			// Act + Assert:
			expect(() => context.view.shrink(6)).to.throw('invalid shrink value');
		});
	});

	// endregion
});
