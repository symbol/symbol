package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.Serializer;

final class ArrayHelpersTest {
	// region helpers

	private static final class MockElement implements Serializer {
		private final int size;
		private final byte marker;

		MockElement(final int size) {
			this(size, (byte) (0x80 | (size & 0x7F)));
		}

		MockElement(final int size, final byte marker) {
			this.size = size;
			this.marker = marker;
		}

		@Override
		public int size() {
			return size;
		}

		@Override
		public byte[] serialize() {
			final byte[] payload = new byte[size];
			Arrays.fill(payload, marker);
			return payload;
		}
	}

	/**
	 * Mock deserialized element carrying its size and the buffer offset it was read at (the tag).
	 */
	private static final class TaggedElement implements Serializer {
		final int size;
		final int tag;

		TaggedElement(final int size, final int tag) {
			this.size = size;
			this.tag = tag;
		}

		@Override
		public int size() {
			return size;
		}

		@Override
		public byte[] serialize() {
			throw new UnsupportedOperationException();
		}

		@Override
		public boolean equals(final Object other) {
			if (!(other instanceof TaggedElement rhs))
				return false;

			return size == rhs.size && tag == rhs.tag;
		}

		@Override
		public int hashCode() {
			return size * 31 + tag;
		}

		@Override
		public String toString() {
			return String.format("(size=%d, tag=%d)", size, tag);
		}
	}

	private static final class MockFactory implements ArrayHelpers.Factory<TaggedElement> {
		private final int[] sizes;
		private final int startTag;
		private final int initialLength;
		private int index;

		MockFactory(final int[] sizes, final int startTag, final int initialLength) {
			this.sizes = sizes;
			this.startTag = startTag;
			this.initialLength = initialLength;
		}

		@Override
		public TaggedElement deserialize(final byte[] buffer, final int offset) {
			final int tag = startTag + offset;
			return new TaggedElement(sizes[index++], tag);
		}
	}

	private static byte[] makeSubView(final int viewSize) {
		// window at offset 15 — only positions matter, observed via the mock factory
		return new byte[viewSize];
	}

	private static List<MockElement> makeElements(final int[] sizes) {
		final List<MockElement> elements = new ArrayList<>(sizes.length);
		for (int size : sizes)
			elements.add(new MockElement(size));
		return elements;
	}

	private static List<MockElement> makeDefaultElements() {
		// element sizes 1, 4, 7, 10, 13
		final int[] sizes = new int[5];
		for (int i = 0; i < sizes.length; ++i)
			sizes[i] = i * 3 + 1;
		return makeElements(sizes);
	}

	private static byte[] expectedSerializedSlice(final List<MockElement> elements, final int upTo) {
		int total = 0;
		for (int i = 0; i < upTo; ++i)
			total += elements.get(i).size;
		final byte[] expected = new byte[total];
		int offset = 0;
		for (int i = 0; i < upTo; ++i) {
			final byte[] data = elements.get(i).serialize();
			System.arraycopy(data, 0, expected, offset, data.length);
			offset += data.length;
		}
		return expected;
	}

	private static byte[] sliceWritten(final Writer writer) {
		return Arrays.copyOf(writer.storage, writer.offset);
	}

	// endregion

	// region alignUp

	@Nested
	final class AlignUp {
		private void assertAlignUp(final int lo, final int hi, final int alignment, final int expectedValue) {
			for (int i = lo; i <= hi; ++i)
				assertThat(ArrayHelpers.alignUp(i, alignment), equalTo(expectedValue));
		}

		@Test
		void alwaysAlignsUp() {
			// Act + Assert:
			assertAlignUp(0, 0, 8, 0);
			assertAlignUp(1, 8, 8, 8);
			assertAlignUp(9, 16, 8, 16);
			assertAlignUp(257, 264, 8, 264);
		}

		@Test
		void canAlignUsingCustomAlignment() {
			// Act + Assert:
			assertAlignUp(0, 0, 11, 0);
			assertAlignUp(1, 11, 11, 11);
			assertAlignUp(12, 22, 11, 22);
			assertAlignUp(353, 363, 11, 363);
		}
	}

	// endregion

	// region size

	@Nested
	final class Size {
		private void assertSize(final int[] sizes, final int expectedSize, final int alignment, final boolean skipLastElementPadding) {
			final List<MockElement> elements = makeElements(sizes);
			final int elementsSize = ArrayHelpers.size(elements, alignment, skipLastElementPadding);
			assertThat(elementsSize, equalTo(expectedSize));
		}

		private void assertSizeAligned(final int[] sizes, final int expectedSize) {
			assertSize(sizes, expectedSize, 9, false);
		}

		private void assertSizeAlignedExLast(final int[] sizes, final int expectedSize) {
			assertSize(sizes, expectedSize, 9, true);
		}

		@Test
		void returnsSumOfSizes() {
			// Act + Assert:
			assertSize(new int[]{}, 0, 0, false);
			assertSize(new int[]{
					13
			}, 13, 0, false);
			assertSize(new int[]{
					13, 21
			}, 34, 0, false);
			assertSize(new int[]{
					13, 21, 34
			}, 68, 0, false);
		}

		@Test
		void returnsSumOfAlignedSizes() {
			// Act + Assert:
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
		void returnsSumOfAlignedSizesExLast() {
			// Act + Assert:
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
	}

	// endregion

	// region read traits

	/**
	 * Shared assertions for the read* helpers; accessor-driven ordering cases are omitted because the Java API has no accessor parameter
	 * (sort ordering is enforced upstream).
	 */
	private static void runReadTraits(final int[] sizes, final int viewSize, final List<TaggedElement> expectedElements,
			final ReadInvoker readInvoker) {
		final byte[] subView = makeSubView(viewSize);
		final MockFactory factory = new MockFactory(sizes, 15, viewSize);
		assertThat(readInvoker.invoke(subView, factory), equalTo(expectedElements));
	}

	@FunctionalInterface
	private interface ReadInvoker {
		List<TaggedElement> invoke(byte[] subView, ArrayHelpers.Factory<TaggedElement> factory);
	}

	private static void assertThrowsWhenAnyElementHasZeroSize(final ReadInvoker readInvoker) {
		final int[] sizes = {
				10, 11, 0, 1, 1
		};
		final byte[] subView = makeSubView(52);
		final MockFactory factory = new MockFactory(sizes, 15, 52);
		final IndexOutOfBoundsException ex = assertThrows(IndexOutOfBoundsException.class, () -> readInvoker.invoke(subView, factory));
		assertThat(ex.getMessage(), equalTo("element size has invalid size"));
	}

	// endregion

	// region readArray

	@Nested
	final class ReadArray {
		@Test
		void throwsWhenAnyElementHasZeroSize() {
			// Act + Assert:
			assertThrowsWhenAnyElementHasZeroSize((subView, factory) -> ArrayHelpers.readArray(subView, factory));
		}

		@Test
		void traitBasedReaderTests() {
			// Act + Assert:
			runReadTraits(new int[]{
					10, 11, 12, 13, 6
			}, 52, List.of(new TaggedElement(10, 15), new TaggedElement(11, 25), new TaggedElement(12, 36), new TaggedElement(13, 48),
					new TaggedElement(6, 61)), (subView, factory) -> ArrayHelpers.readArray(subView, factory));
		}
	}

	// endregion

	// region readArrayCount

	@Nested
	final class ReadArrayCount {
		@Test
		void throwsWhenAnyElementHasZeroSize() {
			// Act + Assert:
			assertThrowsWhenAnyElementHasZeroSize((subView, factory) -> ArrayHelpers.readArrayCount(subView, factory, 3));
		}

		@Test
		void traitBasedReaderTests() {
			// Act + Assert:
			runReadTraits(new int[]{
					10, 11, 12, 43, 79
			}, 52, List.of(new TaggedElement(10, 15), new TaggedElement(11, 25), new TaggedElement(12, 36)),
					(subView, factory) -> ArrayHelpers.readArrayCount(subView, factory, 3));
		}
	}

	// endregion

	// region readVariableSizeElements

	@Nested
	final class ReadVariableSizeElements {
		@Test
		void throwsWhenAnyElementHasZeroSize() {
			// Arrange:
			final int[] sizes = {
					10, 11, 0, 1, 1
			};
			final byte[] subView = makeSubView(52);
			final MockFactory factory = new MockFactory(sizes, 15, 52);

			// Act:
			final IndexOutOfBoundsException ex = assertThrows(IndexOutOfBoundsException.class,
					() -> ArrayHelpers.readVariableSizeElements(subView, factory, 4));

			// Assert:
			assertThat(ex.getMessage(), equalTo("element size has invalid size"));
		}

		@Test
		void readsAllAvailableElements() {
			// Arrange: aligned sizes 8, 12, 12, 16, 4
			final int[] sizes = {
					7, 11, 12, 13, 3
			};
			final byte[] subView = makeSubView(52);
			final MockFactory factory = new MockFactory(sizes, 15, 52);
			final List<TaggedElement> expectedElements = List.of(new TaggedElement(7, 15), new TaggedElement(11, 15 + 8),
					new TaggedElement(12, 15 + 8 + 12), new TaggedElement(13, 15 + 8 + 12 + 12),
					new TaggedElement(3, 15 + 8 + 12 + 12 + 16));

			// Act:
			final List<TaggedElement> elements = ArrayHelpers.readVariableSizeElements(subView, factory, 4);

			// Assert:
			assertThat(elements, equalTo(expectedElements));
		}

		@Test
		void cannotReadAtBufferEndWhenLastReadResultsInOob() {
			// Arrange: aligned sizes: 24, 28
			final int[] sizes = {
					23, 25
			};
			final byte[] subView = makeSubView(49);
			final MockFactory factory = new MockFactory(sizes, 15, 49);

			// Sanity: use same context, but readArray
			{
				final int[] sanitySizes = {
						24, 25
				};
				final byte[] subView2 = makeSubView(49);
				final MockFactory factory2 = new MockFactory(sanitySizes, 15, 49);
				final List<TaggedElement> elements = ArrayHelpers.readArray(subView2, factory2);
				assertThat(elements, equalTo(List.of(new TaggedElement(24, 15), new TaggedElement(25, 15 + 24))));
			}

			final IndexOutOfBoundsException ex = assertThrows(IndexOutOfBoundsException.class,
					() -> ArrayHelpers.readVariableSizeElements(subView, factory, 4));
			assertThat(ex.getMessage(), equalTo("unexpected buffer length"));
		}

		@Test
		void canReadAtBufferEndWhenLastElementPaddingIsSkipped() {
			// Arrange: aligned sizes: 24, 25
			final int[] sizes = {
					23, 25
			};
			final byte[] subView = makeSubView(49);
			final MockFactory factory = new MockFactory(sizes, 15, 49);
			final List<TaggedElement> expectedElements = List.of(new TaggedElement(23, 15), new TaggedElement(25, 15 + 24));

			// Act:
			final List<TaggedElement> elements = ArrayHelpers.readVariableSizeElements(subView, factory, 4, true);

			// Assert:
			assertThat(elements, equalTo(expectedElements));
		}

		@Test
		void cannotReadAtBufferEndWhenLastElementPaddingIsSkippedAndLastReadResultsInOob() {
			// Arrange: aligned sizes: 24, 25
			final int[] sizes = {
					23, 25
			};
			final byte[] subView = makeSubView(48);
			final MockFactory factory = new MockFactory(sizes, 15, 48);

			final IndexOutOfBoundsException ex = assertThrows(IndexOutOfBoundsException.class,
					() -> ArrayHelpers.readVariableSizeElements(subView, factory, 4));
			assertThat(ex.getMessage(), equalTo("unexpected buffer length"));
		}
	}

	// endregion

	// region write traits

	@FunctionalInterface
	private interface WriteInvoker {
		void invoke(Writer output, List<MockElement> elements);
	}

	private static void runWriteTraits(final WriteInvoker writeInvoker, final byte[] expectedBytes) {
		final List<MockElement> elements = makeDefaultElements();
		final Writer output = new Writer(1024);
		writeInvoker.invoke(output, elements);
		assertThat(sliceWritten(output), equalTo(expectedBytes));
	}

	// endregion

	// region writeArray

	@Nested
	final class WriteArray {
		@Test
		void traitBasedWriterTests() {
			// Arrange:
			final byte[] expected = expectedSerializedSlice(makeDefaultElements(), 5);

			// Act + Assert:
			runWriteTraits((output, elements) -> ArrayHelpers.writeArray(output, elements), expected);
		}
	}

	// endregion

	// region sorted-order validation

	@Nested
	final class OrderValidation {
		private final java.util.Comparator<MockElement> bySize = java.util.Comparator.comparingInt(MockElement::size);

		@Test
		void writeArrayAcceptsSortedElements() {
			// Arrange:
			final Writer output = new Writer(1 + 2 + 3);

			// Act:
			ArrayHelpers.writeArray(output, java.util.List.of(new MockElement(1), new MockElement(2), new MockElement(3)), bySize);
		}

		@Test
		void writeArrayRejectsUnsortedElements() {
			// Arrange:
			final Writer output = new Writer(1 + 3 + 2);

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> ArrayHelpers.writeArray(output,
					java.util.List.of(new MockElement(1), new MockElement(3), new MockElement(2)), bySize));

			// Assert:
			assertThat(ex.getMessage(), equalTo("array passed to write array is not sorted"));
		}

		@Test
		void writeArrayRejectsEqualAdjacentElements() {
			// strictly increasing: duplicates are rejected too.
			// Arrange:
			final Writer output = new Writer(2 + 2);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class,
					() -> ArrayHelpers.writeArray(output, java.util.List.of(new MockElement(2), new MockElement(2)), bySize));
		}

		@Test
		void writeArrayCountValidatesOnlyWrittenPrefix() {
			// the unsorted element sits past the count window, so it is neither written nor validated.
			// Arrange:
			final Writer output = new Writer(1 + 2);

			// Act:
			ArrayHelpers.writeArrayCount(output, java.util.List.of(new MockElement(1), new MockElement(2), new MockElement(1)), 2, bySize);
		}

		@Test
		void readArrayCountRejectsUnsortedElements() {
			// elements deserialize with sizes 4, 3 — decreasing, so the read must throw.
			// Arrange:
			final byte[] buffer = new byte[7];
			final java.util.Iterator<Integer> sizes = java.util.List.of(4, 3).iterator();
			final ArrayHelpers.Factory<MockElement> factory = (payload, offset) -> new MockElement(sizes.next());

			// Act:
			final IllegalStateException ex = assertThrows(IllegalStateException.class,
					() -> ArrayHelpers.readArrayCount(buffer, 0, buffer.length, factory, 2, bySize));

			// Assert:
			assertThat(ex.getMessage(), equalTo("elements in array are not sorted"));
		}

		@Test
		void readArrayAcceptsSortedElements() {
			// Arrange:
			final byte[] buffer = new byte[7];
			final java.util.Iterator<Integer> sizes = java.util.List.of(3, 4).iterator();
			final ArrayHelpers.Factory<MockElement> factory = (payload, offset) -> new MockElement(sizes.next());

			// Act:
			final java.util.List<MockElement> elements = ArrayHelpers.readArray(buffer, 0, buffer.length, factory, bySize);

			// Assert:
			assertThat(elements.size(), equalTo(2));
		}
	}

	// endregion

	// region writeArrayCount

	@Nested
	final class WriteArrayCount {
		@Test
		void traitBasedWriterTests() {
			// Arrange:
			final byte[] expected = expectedSerializedSlice(makeDefaultElements(), 3);

			// Act + Assert:
			runWriteTraits((output, elements) -> ArrayHelpers.writeArrayCount(output, elements, 3), expected);
		}
	}

	// endregion

	// region writeVariableSizeElements

	@Nested
	final class WriteVariableSizeElements {
		@Test
		void writesAllElementsAndAligns() {
			// Arrange: sizes 1, 4, 7, 10, 13 -> aligned-to-4 sizes 4, 4, 8, 12, 16
			final List<MockElement> elements = makeDefaultElements();
			final Writer output = new Writer(1024);

			// Act:
			ArrayHelpers.writeVariableSizeElements(output, elements, 4);

			// Assert:
			final byte[] expected = new byte[]{
					// element 0 (size 1) + 3 pad
					(byte) 0x81, 0, 0, 0,
					// element 1 (size 4) — already aligned, no pad
					(byte) 0x84, (byte) 0x84, (byte) 0x84, (byte) 0x84,
					// element 2 (size 7) + 1 pad
					(byte) 0x87, (byte) 0x87, (byte) 0x87, (byte) 0x87, (byte) 0x87, (byte) 0x87, (byte) 0x87, 0,
					// element 3 (size 10) + 2 pad
					(byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A,
					(byte) 0x8A, 0, 0,
					// element 4 (size 13) + 3 pad
					(byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D,
					(byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, 0, 0, 0
			};
			assertThat(sliceWritten(output), equalTo(expected));
		}

		@Test
		void exLastElementWritesAllElementsAndAlignsAllExLast() {
			// Arrange: sizes 1, 4, 7, 10, 13
			final List<MockElement> elements = makeDefaultElements();
			final Writer output = new Writer(1024);

			// Act:
			ArrayHelpers.writeVariableSizeElements(output, elements, 4, true);

			// Assert: same as above, but without trailing 3-byte pad after the last element
			final byte[] expected = new byte[]{
					(byte) 0x81, 0, 0, 0, (byte) 0x84, (byte) 0x84, (byte) 0x84, (byte) 0x84, (byte) 0x87, (byte) 0x87, (byte) 0x87,
					(byte) 0x87, (byte) 0x87, (byte) 0x87, (byte) 0x87, 0, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A,
					(byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, (byte) 0x8A, 0, 0, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D,
					(byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D, (byte) 0x8D,
					(byte) 0x8D
			};
			assertThat(sliceWritten(output), equalTo(expected));
		}
	}

	// endregion
}
