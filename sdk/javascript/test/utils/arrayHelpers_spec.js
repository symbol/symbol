const arrayHelpers = require('../../src/utils/arrayHelpers');
const { expect } = require('chai');

describe('arrayHelpers', () => {
	// region alignUp

	describe('alignUp', () => {
		const assertAlignUp = (range, alignment, expectedValue) => {
			for (let i = range[0]; i <= range[1]; ++i)
				expect(arrayHelpers.alignUp(i, alignment)).to.equal(expectedValue);
		};

		it('always aligns up', () => {
			assertAlignUp([0, 0], 8, 0);
			assertAlignUp([1, 8], 8, 8);
			assertAlignUp([9, 16], 8, 16);
			assertAlignUp([257, 264], 8, 264);
		});

		it('can align using custom alignment', () => {
			assertAlignUp([0, 0], 11, 0);
			assertAlignUp([1, 11], 11, 11);
			assertAlignUp([12, 22], 11, 22);
			assertAlignUp([353, 363], 11, 363);
		});
	});

	// endregion

	// region readers

	class ReadTestContext {
		constructor(sizes, viewSize = 52) {
			this.byteArray = new Uint8Array(100);
			this.subView = new Uint8Array(this.byteArray.buffer, 15, viewSize);
			this.factory = {
				sizes,
				index: 0,
				deserialize(buffer) {
					return { size: this.sizes[this.index++], tag: buffer.byteOffset };
				}
			};
		}
	}

	const assertThrowsWhenAnyElementHasZeroSize = reader =>
		it('throws when any element has zero size', () => {
			// Arrange:
			const context = new ReadTestContext([10, 11, 0, 1, 1]);

			// Act + Assert:
			expect(() => reader(context.subView, context.factory)).to.throw('element size has invalid size');
		});

	const addTraitBasedReaderTests = traits => {
		it('reads all available elements', () => {
			// Arrange:
			const context = new ReadTestContext(traits.sizes);

			// Act:
			const elements = traits.read(context.subView, context.factory);

			// Assert:
			expect(elements).to.deep.equal(traits.expectedElements);
		});

		it('can read when using accessor and elements are ordered', () => {
			// Arrange:
			const context = new ReadTestContext(traits.sizes);

			// Act:
			const elements = traits.read(context.subView, context.factory, element => element.tag);

			// Assert:
			expect(elements).to.deep.equal(traits.expectedElements);
		});

		it('cannot read when using accessor and elements are not ordered', () => {
			// Arrange:
			const context = new ReadTestContext(traits.sizes);

			// Act + Assert: use negative to fake wrong ordering
			expect(() => traits.read(context.subView, context.factory, element => -element.tag))
				.to.throw('elements in array are not sorted');
		});
	};

	describe('readArray', () => {
		assertThrowsWhenAnyElementHasZeroSize(arrayHelpers.readArray);

		addTraitBasedReaderTests({
			sizes: [10, 11, 12, 13, 6],
			expectedElements: [
				{ size: 10, tag: 15 },
				{ size: 11, tag: 25 },
				{ size: 12, tag: 36 },
				{ size: 13, tag: 48 },
				{ size: 6, tag: 61 }
			],
			read: arrayHelpers.readArray
		});
	});

	describe('readArrayCount', () => {
		assertThrowsWhenAnyElementHasZeroSize((view, factory) => arrayHelpers.readArrayCount(view, factory, 3));

		addTraitBasedReaderTests({
			sizes: [10, 11, 12, 43, 79],
			expectedElements: [
				{ size: 10, tag: 15 },
				{ size: 11, tag: 25 },
				{ size: 12, tag: 36 }
			],
			read: (view, factory, accessor) => arrayHelpers.readArrayCount(view, factory, 3, accessor)
		});
	});

	describe('readVariableSizeElements', () => {
		assertThrowsWhenAnyElementHasZeroSize((view, factory) => arrayHelpers.readVariableSizeElements(view, factory, 4));

		it('reads all available elements', () => {
			// Arrange: aligned sizes 8, 12, 12, 16, 4
			const context = new ReadTestContext([7, 11, 12, 13, 3]);
			const expectedElements = [
				{ size: 7, tag: 15 },
				{ size: 11, tag: 15 + 8 },
				{ size: 12, tag: 15 + 8 + 12 },
				{ size: 13, tag: 15 + 8 + 12 + 12 },
				{ size: 3, tag: 15 + 8 + 12 + 12 + 16 }
			];

			// Act:
			const elements = arrayHelpers.readVariableSizeElements(context.subView, context.factory, 4);

			// Assert:
			expect(elements).to.deep.equal(expectedElements);
		});

		it('throws when reading would result in OOB read', () => {
			// Arrange:
			const context = new ReadTestContext([24, 25], 49);

			// Sanity: use same context, but readArray
			{
				const context2 = new ReadTestContext([24, 25], 49);
				const elements = arrayHelpers.readArray(context2.subView, context2.factory);
				expect(elements).to.deep.equal([{ size: 24, tag: 15 }, { size: 25, tag: 15 + 24 }]);
			}

			// Act + Assert:
			expect(() => arrayHelpers.readVariableSizeElements(context.subView, context.factory, 4))
				.to.throw('unexpected buffer length');
		});
	});

	// endregion

	// region writers

	class MockElement {
		constructor(size) {
			this.size = size;
		}

		serialize() {
			return 100 + this.size;
		}
	}

	class WriteTestContext {
		constructor() {
			this.elements = [];
			for (let i = 0; 5 > i; ++i)
				this.elements.push(new MockElement((i * 3) + 1));

			this.output = {
				writes: [],
				write(value) {
					this.writes.push(value instanceof Uint8Array ? { type: 'fill', value: value.length } : { type: 'value', value });
				}
			};
		}
	}

	const addTraitBasedWriterTests = traits => {
		it('writes all elements', () => {
			// Arrange:
			const context = new WriteTestContext();

			// Act:
			traits.write(context.output, context.elements);

			// Assert:
			expect(context.output.writes).to.deep.equal(traits.expectedWrites);
		});

		it('can write when using accessor and elements are ordered', () => {
			// Arrange:
			const context = new WriteTestContext();

			// Act:
			traits.write(context.output, context.elements, element => element.size);

			// Assert:
			expect(context.output.writes).to.deep.equal(traits.expectedWrites);
		});

		it('cannot write when using accessor and elements are not ordered', () => {
			// Arrange:
			const context = new WriteTestContext();

			// Act + Assert:
			expect(() => traits.write(context.output, context.elements, element => -element.size))
				.to.throw('array passed to writeArray is not sorted');
		});
	};

	describe('writeArray', () => {
		addTraitBasedWriterTests({
			write: arrayHelpers.writeArray,
			expectedWrites: [
				{ type: 'value', value: 101 },
				{ type: 'value', value: 104 },
				{ type: 'value', value: 107 },
				{ type: 'value', value: 110 },
				{ type: 'value', value: 113 }
			]
		});
	});

	describe('writeArrayCount', () => {
		addTraitBasedWriterTests({
			write: (output, elements, accessor) => arrayHelpers.writeArrayCount(output, elements, 3, accessor),
			expectedWrites: [
				{ type: 'value', value: 101 },
				{ type: 'value', value: 104 },
				{ type: 'value', value: 107 }
			]
		});
	});

	describe('writeVariableSizeElements', () => {
		it('writes all elements and aligns', () => {
			// Arrange:
			const context = new WriteTestContext();

			// Act:
			arrayHelpers.writeVariableSizeElements(context.output, context.elements, 4);

			// Assert:
			expect(context.output.writes).to.deep.equal([
				{ type: 'value', value: 101 },
				{ type: 'fill', value: 3 },
				{ type: 'value', value: 104 },
				// no fill here, because write was aligned
				{ type: 'value', value: 107 },
				{ type: 'fill', value: 1 },
				{ type: 'value', value: 110 },
				{ type: 'fill', value: 2 },
				{ type: 'value', value: 113 },
				{ type: 'fill', value: 3 }
			]);
		});
	});

	// endregion
});
