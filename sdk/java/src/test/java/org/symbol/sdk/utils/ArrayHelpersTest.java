package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;
import org.junit.Test;

public class ArrayHelpersTest {
	// region helpers

	class MockElement implements Serializable {
		final int size;
		public int tag;

		public MockElement(final int size, final int tag) {
			this.size = size;
			this.tag = tag;
		}

		public int getSize() {
			return size;
		}

		public byte[] serialize() {
			return new byte[100 + this.size];
		}

		public int getTag() {
			return tag;
		}
		public void setTag(int tag) {
			this.tag = tag;
		}

		@Override
		public boolean equals(Object obj) {
			if (null == obj) {
				return false;
			}

			if (this == obj) {
				return true;
			}

			if (!(obj instanceof MockElement)) {
				return false;
			}

			final MockElement other = (MockElement) obj;
			return this.size == other.size && this.tag == other.tag;
		}
	}

	class ElementsTestContext {
		final int[] elementSizes;
		final List<Serializable> elements;
		final Writer output;
		final List<String> writes;

		ElementsTestContext() {
			this(new int[]{
					1, 4, 7, 10, 13
			});
		}

		ElementsTestContext(final int[] sizes) {
			this.elementSizes = sizes;
			this.elements = Arrays.stream(this.elementSizes).mapToObj(size -> new MockElement(size, 0)).collect(Collectors.toList());
			this.writes = new ArrayList<>();
			this.output = new Writer() {
				@Override
				public void write(byte[] buffer) {
					final StringBuilder sb = new StringBuilder();
					sb.append("type: ");
					sb.append(buffer.length > 100 ? "value" : "fill");
					sb.append(" value: ").append(buffer.length);
					writes.add(sb.toString());
				}

				@Override
				public byte[] getStorage() {
					return new byte[0];
				}
			};
		}
	}

	// endregion

	// region deepCompare

	@Test
	public void deepCompareCanComparePrimitive() {
		// Act + Assert:
		assertThat(ArrayHelpers.deepCompare(12, 15), equalTo(-1));
		assertThat(ArrayHelpers.deepCompare(15, 15), equalTo(0));
		assertThat(ArrayHelpers.deepCompare(17, 15), equalTo(1));
	}

	@Test
	public void deepCompareCanCompareArrays() {
		// Act + Assert:
		assertThat(ArrayHelpers.deepCompare(new Integer[]{
				1, 12, 3
		}, new Integer[]{
				1, 15, 3
		}), equalTo(-1));
		assertThat(ArrayHelpers.deepCompare(new Integer[]{
				1, 15, 3
		}, new Integer[]{
				1, 15, 3
		}), equalTo(0));
		assertThat(ArrayHelpers.deepCompare(new Integer[]{
				1, 17, 3
		}, new Integer[]{
				1, 15, 3
		}), equalTo(1));
	}

	@Test
	public void deepCompareCanCompareDifferentLengthArrays() {
		// Act + Assert:
		assertThat(ArrayHelpers.deepCompare(new Integer[]{
				1, 12, 3
		}, new Integer[]{
				1, 12, 3, 4
		}), equalTo(-1));
		assertThat(ArrayHelpers.deepCompare(new Integer[]{
				1, 12, 3, 4
		}, new Integer[]{
				1, 12, 3, 4
		}), equalTo(0));
		assertThat(ArrayHelpers.deepCompare(new Integer[]{
				1, 12, 3, 4, 16
		}, new Integer[]{
				1, 12, 3, 4
		}), equalTo(1));
	}

	// endregion

	// region alignUp

	private void assertAlignUp(final int[] range, final int alignment, final int expectedValue) {
		for (int i = range[0]; i <= range[1]; ++i)
			assertThat(ArrayHelpers.alignUp(i, alignment), equalTo(expectedValue));
	}

	@Test
	public void canAlwaysAlignsUp() {
		assertAlignUp(new int[]{
				0, 0
		}, 8, 0);
		assertAlignUp(new int[]{
				1, 8
		}, 8, 8);
		assertAlignUp(new int[]{
				9, 16
		}, 8, 16);
		assertAlignUp(new int[]{
				257, 264
		}, 8, 264);
	}

	@Test
	public void canAlignCustomAlignment() {
		assertAlignUp(new int[]{
				0, 0
		}, 11, 0);
		assertAlignUp(new int[]{
				1, 11
		}, 11, 11);
		assertAlignUp(new int[]{
				12, 22
		}, 11, 22);
		assertAlignUp(new int[]{
				353, 363
		}, 11, 363);
	}

	// endregion

	// region size

	private void assertSize(final int[] sizes, final int expectedSize, final int alignment, final boolean skipLastElementPadding) {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext(sizes);

		// Act:
		final int elementsSize = ArrayHelpers.size(context.elements, alignment, skipLastElementPadding);

		// Assert:
		assertThat(elementsSize, equalTo(expectedSize));
	}

	private void assertSize(final int[] sizes, final int expectedSize, final int alignment) {
		assertSize(sizes, expectedSize, alignment, false);
	}

	private void assertSize(final int[] sizes, final int expectedSize) {
		assertSize(sizes, expectedSize, 0, false);
	}

	private void assertSizeAligned(final int[] sizes, final int expectedSize) {
		assertSize(sizes, expectedSize, 9);
	}

	private void assertSizeAlignedExLast(final int[] sizes, final int expected_size) {
		assertSize(sizes, expected_size, 9, true);
	}

	@Test
	public void canReturnSumOfSizes() {
		assertSize(new int[]{}, 0);
		assertSize(new int[]{
				13
		}, 13);
		assertSize(new int[]{
				13, 21
		}, 34);
		assertSize(new int[]{
				13, 21, 34
		}, 68);
	}

	@Test
	public void canReturnSumOfAlignedSizes() {
		assertSizeAligned(new int[]{}, 0);
		assertSizeAligned(new int[]{
				1
		}, 9);
		assertSizeAligned(new int[]{
				13
		}, 18);
		assertSizeAligned(new int[]{
				13, 21
		}, 18 + 27);
		assertSizeAligned(new int[]{
				13, 21, 34
		}, 18 + 27 + 36);
	}

	@Test
	public void canReturnSumOfAlignedSizesExcludeLast() {
		assertSizeAlignedExLast(new int[]{}, 0);
		assertSizeAlignedExLast(new int[]{
				1
		}, 1);
		assertSizeAlignedExLast(new int[]{
				13
		}, 13);
		assertSizeAlignedExLast(new int[]{
				13, 21
		}, 18 + 21);
		assertSizeAlignedExLast(new int[]{
				13, 21, 34
		}, 18 + 27 + 34);
	}

	// endregion

	// region readers helpers

	class ReadTestContext {
		final byte[] buffer;
		final ByteBuffer subView;
		int index;
		final int[] sizes;

		ReadTestContext(final int[] sizes) {
			this(sizes, 52);
		}

		ReadTestContext(final int[] sizes, final int viewSize) {
			this.buffer = new byte[100];
			this.index = 0;
			this.subView = ByteBuffer.wrap(this.buffer, 15, viewSize);
			this.sizes = sizes;
		}

		final Serializable factory(final ByteBuffer buffer) {
			final BufferView view = BufferView.wrap(buffer);
			final int size = this.sizes[this.index++];
			return new MockElement(size, view.getBuffer().arrayOffset() + view.getBuffer().position());
		};
	}

	// endregion

	// region ReadArray

	@Test
	public void readArrayCanThrowsWhenAnyElementHasZeroSize() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(new int[]{
				10, 11, 0, 1, 1
		});

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readArray(context.subView, context::factory));
		assertThat(thrown.getMessage(), equalTo("element size has invalid size"));
	}

	final int[] readArraySizes = new int[]{
			10, 11, 12, 13, 6
	};
	final List<MockElement> readArrayExpectedElement = Arrays.asList(new MockElement(10, 15), new MockElement(11, 25),
			new MockElement(12, 36), new MockElement(13, 48), new MockElement(6, 61));

	@Test
	public void readArrayCanReadsAllAvailableElements() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArraySizes);

		// Act:
		final List<Serializable> elements = ArrayHelpers.readArray(context.subView, context::factory);

		// Assert:
		assertThat(elements, equalTo(readArrayExpectedElement));
	}

	@Test
	public void canReadArrayWhenUsingAccessorAndElementAreOrdered() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArraySizes);

		// Act:
		final List<Serializable> elements = ArrayHelpers.readArray(context.subView, context::factory, element -> new Integer[]{
				((MockElement) element).getTag()
		});

		// Assert:
		assertThat(elements, equalTo(readArrayExpectedElement));
	}

	@Test
	public void cannotReadArrayWhenUsingAccessorAndElementAreUnOrdered() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArraySizes);

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readArray(context.subView, context::factory, element -> new Integer[]{
						-((MockElement) element).getTag()
				}));
		assertThat(thrown.getMessage(), equalTo("elements in array are not sorted"));
	}

	// endregion

	// region ReadArrayCount

	@Test
	public void readArrayCountCanThrowWhenAnyElementHasZeroSize() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(new int[]{
				10, 0, 5, 1, 1
		});

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readArrayCount(context.subView, context::factory, 5));
		assertThat(thrown.getMessage(), equalTo("element size has invalid size"));
	}

	final int[] readArrayCountSizes = new int[]{
			10, 11, 12, 43, 79
	};
	final List<MockElement> readArrayCountExpectedElement = Arrays.asList(new MockElement(10, 15), new MockElement(11, 25),
			new MockElement(12, 36));

	@Test
	public void readArrayCountCanReadsAllAvailableElements() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArrayCountSizes);

		// Act:
		final List<Serializable> elements = ArrayHelpers.readArrayCount(context.subView, context::factory, 3);

		// Assert:
		assertThat(elements, equalTo(readArrayCountExpectedElement));
	}

	@Test
	public void canReadArrayCountWhenUsingAccessorAndElementAreOrdered() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArrayCountSizes);

		// Act:
		final List<Serializable> elements = ArrayHelpers.readArrayCount(context.subView, context::factory, 3, element -> new Integer[]{
				((MockElement) element).getTag()
		});

		// Assert:
		assertThat(elements, equalTo(readArrayCountExpectedElement));
	}

	@Test
	public void canReadArrayCountWhenUsingAccessorAndArrayElementAreOrdered() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArrayCountSizes);

		// Act:
		final List<Serializable> elements = ArrayHelpers.readArrayCount(context.subView, context::factory, 3, element -> new Integer[]{
				123, 1, ((MockElement) element).getTag(), 3
		});

		// Assert:
		assertThat(elements, equalTo(readArrayCountExpectedElement));
	}

	@Test
	public void cannotReadArrayCountWhenUsingAccessorAndElementAreUnOrdered() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(readArrayCountSizes);

		// Act:
		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readArrayCount(context.subView, context::factory, 3, element -> new Integer[]{
						-((MockElement) element).getTag()
				}));
		assertThat(thrown.getMessage(), equalTo("elements in array are not sorted"));
	}

	// endregion

	// region readVariableSize

	@Test
	public void readVariableSizeCanThrowsWhenAnyElementHasZeroSize() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(new int[]{
				10, 11, 0, 1, 1
		});

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readVariableSizeElements(context.subView, context::factory, 8));
		assertThat(thrown.getMessage(), equalTo("element size has invalid size"));
	}

	@Test
	public void canReadVariableSizeReadsAllAvailableElements() {
		// Arrange: aligned sizes 8, 12, 12, 16, 4
		final ReadTestContext context = new ReadTestContext(new int[]{
				7, 11, 12, 13, 3
		});
		final List<MockElement> expectedElements = Arrays.asList(new MockElement(7, 15), new MockElement(11, 15 + 8),
				new MockElement(12, 15 + 8 + 12), new MockElement(13, 15 + 8 + 12 + 12), new MockElement(3, 15 + 8 + 12 + 12 + 16));

		// Act:
		final List<Serializable> elements = ArrayHelpers.readVariableSizeElements(context.subView, context::factory, 4);

		// Assert:
		assertThat(elements, equalTo(expectedElements));
	}

	@Test
	public void cannotReadAtBufferEndWhenLastReadResultsInOOB() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(new int[]{
				23, 25
		}, 49);

		// Sanity: use same context, but readArray
		{
			final ReadTestContext context2 = new ReadTestContext(new int[]{
					24, 25
			}, 49);
			final List<Serializable> elements = ArrayHelpers.readArray(context2.subView, context2::factory);
			final List<MockElement> readArrayExpectedElement = Arrays.asList(new MockElement(24, 15), new MockElement(25, 15 + 24));

			assertThat(elements, equalTo(readArrayExpectedElement));
		}

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readVariableSizeElements(context.subView, context::factory, 4));
		assertThat(thrown.getMessage(), equalTo("unexpected buffer length"));
	}

	@Test
	public void canReadAtBufferEndWhenLastElementPaddingIsSkipped() {
		// Arrange: aligned sizes: 24, 25
		final ReadTestContext context = new ReadTestContext(new int[]{
				23, 25
		}, 49);
		final List<MockElement> readArrayExpectedElement = Arrays.asList(new MockElement(23, 15), new MockElement(25, 15 + 24));

		// Act:
		final List<Serializable> elements = ArrayHelpers.readVariableSizeElements(context.subView, context::factory, 4, true);

		// Assert:
		assertThat(elements, equalTo(readArrayExpectedElement));
	}

	@Test
	public void cannotReadAtBufferEndWhenLastElementPaddingIsSkippedAndLastReadResultsInOOB() {
		// Arrange:
		final ReadTestContext context = new ReadTestContext(new int[]{
				23, 25
		}, 48);

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.readVariableSizeElements(context.subView, context::factory, 4, true));
		assertThat(thrown.getMessage(), equalTo("unexpected buffer length"));
	}

	// endregion

	// region writeArray

	final List<String> writeArrayExpectedWrites = new ArrayList<>() {
		{
			add("type: value value: 101");
			add("type: value value: 104");
			add("type: value value: 107");
			add("type: value value: 110");
			add("type: value value: 113");
		}
	};

	@Test
	public void writeArrayCanWritesAllElements() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();

		// Act:
		ArrayHelpers.writeArray(context.output, context.elements);

		// Assert:
		assertThat(context.writes, equalTo(writeArrayExpectedWrites));
	}

	@Test
	public void writeArrayCanWriteUsingAccessorAndElementsAreOrdered() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();

		// Act:
		ArrayHelpers.writeArray(context.output, context.elements, element -> new Integer[]{
				element.getSize()
		});

		// Assert:
		assertThat(context.writes, equalTo(writeArrayExpectedWrites));
	}

	@Test
	public void writeArrayCanWriteUsingAccessorAndElementsAreNotOrdered() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.writeArray(context.output, context.elements, element -> new Integer[]{
						-element.getSize()
				}));
		assertThat(thrown.getMessage(), equalTo("array passed to write array is not sorted"));
	}

	// endregion

	// region writeArrayCount

	final List<String> writeArrayCountExpectedWrites = new ArrayList<>() {
		{
			add("type: value value: 101");
			add("type: value value: 104");
			add("type: value value: 107");
		}
	};

	@Test
	public void writeArrayCountCanWritesAllElements() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();

		// Act:
		ArrayHelpers.writeArrayCount(context.output, context.elements, 3);

		// Assert:
		assertThat(context.writes, equalTo(writeArrayCountExpectedWrites));
	}

	@Test
	public void writeArrayCountCanWriteUsingAccessorAndElementsAreOrdered() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();

		// Act:
		ArrayHelpers.writeArrayCount(context.output, context.elements, 3, element -> new Integer[]{
				element.getSize()
		});

		// Assert:
		assertThat(context.writes, equalTo(writeArrayCountExpectedWrites));
	}

	@Test
	public void writeArrayCountCanWriteUsingAccessorAndElementsAreNotOrdered() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();

		// Act + Assert:
		final IllegalStateException thrown = assertThrows(IllegalStateException.class,
				() -> ArrayHelpers.writeArrayCount(context.output, context.elements, 3, element -> new Integer[]{
						-element.getSize()
				}));
		assertThat(thrown.getMessage(), equalTo("array passed to write array is not sorted"));
	}

	// endregion

	// region writeVariableSizeElements

	@Test
	public void writeVariableSizeElementsCanWritesAllElementsAndAligns() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();
		final List<String> writeVariableSizeExpectedWrites = new ArrayList<>() {
			{
				add("type: value value: 101");
				add("type: fill value: 3");
				add("type: value value: 104");
				// no fill here, because write was aligned
				add("type: value value: 107");
				add("type: fill value: 1");
				add("type: value value: 110");
				add("type: fill value: 2");
				add("type: value value: 113");
				add("type: fill value: 3");
			}
		};

		// Act:
		ArrayHelpers.writeVariableSizeElements(context.output, context.elements, 4);

		assertThat(context.writes, equalTo(writeVariableSizeExpectedWrites));
	}

	@Test
	public void writeVariableSizeCanExLastElementsWritesAllElementsAndAlignsAllExLast() {
		// Arrange:
		final ElementsTestContext context = new ElementsTestContext();
		final List<String> writeVariableSizeExpectedWrites = new ArrayList<>() {
			{
				add("type: value value: 101");
				add("type: fill value: 3");
				add("type: value value: 104");
				// no fill here, because write was aligned
				add("type: value value: 107");
				add("type: fill value: 1");
				add("type: value value: 110");
				add("type: fill value: 2");
				add("type: value value: 113");
			}
		};

		// Act:
		ArrayHelpers.writeVariableSizeElements(context.output, context.elements, 4, true);

		assertThat(context.writes, equalTo(writeVariableSizeExpectedWrites));
	}

	// endregion
}
