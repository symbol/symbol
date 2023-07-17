import * as arrayHelpers from '../../src/utils/arrayHelpers.js';
import { expect } from 'chai';

describe('arrayHelpers', () => {
	// region helpers

	class MockElement {
		constructor(size) {
			this.size = size;
		}

		serialize() {
			return 100 + this.size;
		}
	}

	class ElementsTestContext {
		constructor(sizes = undefined) {
			const elementSizes = sizes || Array.from({ length: 5 }, (_, index) => (index * 3) + 1);
			this.elements = elementSizes.map(size => new MockElement(size));

			this.output = {
				/**
				 * @type {Array<{type: string, value: number}>}
				 */
				writes: [],
				write(value) {
					this.writes.push(value instanceof Uint8Array ? { type: 'fill', value: value.length } : { type: 'value', value });
				}
			};
		}
	}

	// endregion

	// region deepCompare

	describe('deepCompare', () => {
		it('can compare primitive', () => {
			expect(arrayHelpers.deepCompare(12, 15)).to.equal(-1);
			expect(arrayHelpers.deepCompare(15, 15)).to.equal(0);
			expect(arrayHelpers.deepCompare(17, 15)).to.equal(1);
		});

		it('can compare array', () => {
			expect(arrayHelpers.deepCompare([1, 12, 3], [1, 15, 3])).to.equal(-1);
			expect(arrayHelpers.deepCompare([1, 15, 3], [1, 15, 3])).to.equal(0);
			expect(arrayHelpers.deepCompare([1, 17, 3], [1, 15, 3])).to.equal(1);
		});

		it('can compare arrays of different lengths', () => {
			expect(arrayHelpers.deepCompare([1, 12, 3], [1, 12, 3, 4])).to.equal(-1);
			expect(arrayHelpers.deepCompare([1, 12, 3, 4], [1, 12, 3, 4])).to.equal(0);
			expect(arrayHelpers.deepCompare([1, 12, 3, 4, 16], [1, 12, 3, 4])).to.equal(1);
		});
	});

	// endregion

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

	// region size

	describe('size', () => {
		const assertSize = (sizes, expectedSize, alignment = 0, skipLastElementPadding = false) => {
			// Arrange:
			const context = new ElementsTestContext(sizes);

			// Act:
			const elementsSize = arrayHelpers.size(context.elements, alignment, skipLastElementPadding);

			// Assert:
			expect(elementsSize).to.equal(expectedSize);
		};

		const assertSizeAligned = (sizes, expectedSize) => {
			assertSize(sizes, expectedSize, 9);
		};

		const assertSizeAlignedExLast = (sizes, expected_size) => {
			assertSize(sizes, expected_size, 9, true);
		};

		it('returns sum of sizes', () => {
			assertSize([], 0);
			assertSize([13], 13);
			assertSize([13, 21], 34);
			assertSize([13, 21, 34], 68);
		});

		it('returns sum of aligned sizes', () => {
			assertSizeAligned([], 0);
			assertSizeAligned([1], 9);
			assertSizeAligned([13], 18);
			assertSizeAligned([13, 21], 18 + 27);
			assertSizeAligned([13, 21, 34], 18 + 27 + 36);
		});

		it('returns sum of aligned sizes ex last', () => {
			assertSizeAlignedExLast([], 0);
			assertSizeAlignedExLast([1], 1);
			assertSizeAlignedExLast([13], 13);
			assertSizeAlignedExLast([13, 21], 18 + 21);
			assertSizeAlignedExLast([13, 21, 34], 18 + 27 + 34);
		});
	});

	// endregion

	// region readers

	class ReadTestContext {
		constructor(sizes, viewSize = 52) {
			this.buffer = new Uint8Array(100);
			this.subView = new Uint8Array(this.buffer.buffer, 15, viewSize);
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

		it('can read when using accessor and (array) elements are ordered', () => {
			// Arrange:
			const context = new ReadTestContext(traits.sizes);

			// Act:
			const elements = traits.read(context.subView, context.factory, element => [123, new Uint8Array([1, element.tag, 3])]);

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

		it('cannot read at buffer end when last read results in OOB', () => {
			// Arrange: aligned sizes: 24, 28
			const context = new ReadTestContext([23, 25], 49);

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

		it('can read at buffer end when last element padding is skipped', () => {
			// Arrange: aligned sizes: 24, 25
			const context = new ReadTestContext([23, 25], 49);
			const expectedElements = [
				{ size: 23, tag: 15 },
				{ size: 25, tag: 15 + 24 }
			];

			// Act:
			const elements = arrayHelpers.readVariableSizeElements(context.subView, context.factory, 4, true);

			// Assert:
			expect(elements).to.deep.equal(expectedElements);
		});

		it('cannot read at buffer end when last element padding is skipped and last read results in OOB', () => {
			// Arrange: aligned sizes: 24, 25
			const context = new ReadTestContext([23, 25], 48);

			// Act + Assert:
			expect(() => arrayHelpers.readVariableSizeElements(context.subView, context.factory, 4))
				.to.throw('unexpected buffer length');
		});
	});

	// endregion

	// region writers

	const addTraitBasedWriterTests = traits => {
		it('writes all elements', () => {
			// Arrange:
			const context = new ElementsTestContext();

			// Act:
			traits.write(context.output, context.elements);

			// Assert:
			expect(context.output.writes).to.deep.equal(traits.expectedWrites);
		});

		it('can write when using accessor and elements are ordered', () => {
			// Arrange:
			const context = new ElementsTestContext();

			// Act:
			traits.write(context.output, context.elements, element => element.size);

			// Assert:
			expect(context.output.writes).to.deep.equal(traits.expectedWrites);
		});

		it('can write when using accessor and (array) elements are ordered', () => {
			// Arrange:
			const context = new ElementsTestContext();

			// Act:
			traits.write(context.output, context.elements, element => [123, new Uint8Array([1, element.size, 3])]);

			// Assert:
			expect(context.output.writes).to.deep.equal(traits.expectedWrites);
		});

		it('cannot write when using accessor and elements are not ordered', () => {
			// Arrange:
			const context = new ElementsTestContext();

			// Act + Assert:
			expect(() => traits.write(context.output, context.elements, element => -element.size))
				.to.throw('array passed to write array is not sorted');
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
			const context = new ElementsTestContext();

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

		it('ex last element writes all elements and aligns all ex last', () => {
			// Arrange:
			const context = new ElementsTestContext();

			// Act:
			arrayHelpers.writeVariableSizeElements(context.output, context.elements, 4, true);

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
				{ type: 'value', value: 113 }
			]);
		});
	});

	// endregion
});
