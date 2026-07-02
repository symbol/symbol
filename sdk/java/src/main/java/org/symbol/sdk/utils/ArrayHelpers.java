package org.symbol.sdk.utils;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

import org.symbol.sdk.Serializer;

/**
 * Helpers for reading / writing arrays of {@link Serializer} objects.
 */
public final class ArrayHelpers {
	private ArrayHelpers() {
	}

	/**
	 * Factory used to deserialize objects from a byte slice.
	 *
	 * @param <T> Element type.
	 */
	@FunctionalInterface
	public interface Factory<T extends Serializer> {
		/**
		 * Deserializes a single element from the current position of the given view. The view is not advanced — the caller moves the cursor
		 * by the returned element's {@code size()}.
		 *
		 * @param view Buffer view positioned at the element.
		 * @return Deserialized element.
		 */
		T deserialize(BufferView view);
	}

	private interface ContinuationPredicate {
		boolean shouldContinue(int index, BufferView cursor);
	}

	private static <T extends Serializer> List<T> readArrayImpl(final BufferView view, final Factory<T> factoryClass,
			final Comparator<? super T> comparator, final ContinuationPredicate shouldContinue) {
		final BufferView cursor = view.snapshot();
		final List<T> elements = new ArrayList<>();
		T previousElement = null;
		int i = 0;
		while (shouldContinue.shouldContinue(i, cursor)) {
			final T element = factoryClass.deserialize(cursor);
			final int elementSize = element.size();

			if (0 >= elementSize)
				throw new IndexOutOfBoundsException("element size has invalid size");

			if (null != comparator && null != previousElement && 0 <= comparator.compare(previousElement, element))
				throw new IllegalStateException("elements in array are not sorted");

			elements.add(element);
			cursor.shiftRight(elementSize);
			previousElement = element;
			++i;
		}

		return elements;
	}

	private static <T extends Serializer> void writeArrayImpl(final Writer output, final List<T> elements, final int count,
			final Comparator<? super T> comparator) {
		for (int i = 0; i < count; ++i) {
			final T element = elements.get(i);
			if (null != comparator && 0 < i && 0 <= comparator.compare(elements.get(i - 1), element))
				throw new IllegalArgumentException("array passed to write array is not sorted");

			output.write(element.serialize());
		}
	}

	/**
	 * Calculates aligned size.
	 *
	 * @param size Size.
	 * @param alignment Alignment.
	 * @return Size rounded up to alignment.
	 */
	public static int alignUp(final int size, final int alignment) {
		return Math.ceilDiv(size, alignment) * alignment;
	}

	/**
	 * Calculates size of variable size objects.
	 *
	 * @param elements Serializable elements.
	 * @param alignment Alignment used for calculations (0 means no alignment).
	 * @param skipLastElementPadding {@code true} if last element should not be aligned.
	 * @return Computed size.
	 */
	public static int size(final List<? extends Serializer> elements, final int alignment, final boolean skipLastElementPadding) {
		if (elements.isEmpty())
			return 0;

		int total = 0;
		final int count = elements.size();

		// Loop through all elements except the last one
		for (int i = 0; i < count - 1; ++i) {
			final int size = elements.get(i).size();
			total += (alignment == 0) ? size : alignUp(size, alignment);
		}

		// Process the final element explicitly
		final int lastSize = elements.get(count - 1).size();
		if (alignment == 0 || skipLastElementPadding) {
			total += lastSize; // No padding applied to the final element
		} else {
			total += alignUp(lastSize, alignment); // Apply padding to the final element
		}

		return total;
	}

	/**
	 * Calculates size of variable size objects with default alignment of 0.
	 *
	 * @param elements Serializable elements.
	 * @return Computed size.
	 */
	public static int size(final List<? extends Serializer> elements) {
		return size(elements, 0, false);
	}

	/**
	 * Reads array of objects until the view's window is exhausted, without order verification.
	 *
	 * @param <T> Element type.
	 * @param view View positioned at the array, whose window bounds the read.
	 * @param factoryClass Factory used to deserialize objects.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArray(final BufferView view, final Factory<T> factoryClass) {
		return readArray(view, factoryClass, null);
	}

	/**
	 * Reads array of objects until the view's window is exhausted, verifying strictly increasing element order.
	 *
	 * @param <T> Element type.
	 * @param view View positioned at the array, whose window bounds the read.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param comparator Sort-key comparator enforcing element order; {@code null} skips the check.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArray(final BufferView view, final Factory<T> factoryClass,
			final Comparator<? super T> comparator) {
		return readArrayImpl(view, factoryClass, comparator, (idx, cursor) -> 0 < cursor.length());
	}

	/**
	 * Reads array of a deterministic number of objects.
	 *
	 * @param <T> Element type.
	 * @param view View positioned at the array, whose window bounds the read.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param count Number of objects to deserialize.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArrayCount(final BufferView view, final Factory<T> factoryClass, final int count) {
		return readArrayCount(view, factoryClass, count, null);
	}

	/**
	 * Reads array of a deterministic number of objects, verifying strictly increasing element order.
	 *
	 * @param <T> Element type.
	 * @param view View positioned at the array, whose window bounds the read.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param count Number of objects to deserialize.
	 * @param comparator Sort-key comparator enforcing element order; {@code null} skips the check.
	 * @return Array of deserialized objects.
	 * @throws IndexOutOfBoundsException if {@code count} is negative (e.g. a corrupt u32 count field that narrowed below zero).
	 */
	public static <T extends Serializer> List<T> readArrayCount(final BufferView view, final Factory<T> factoryClass, final int count,
			final Comparator<? super T> comparator) {
		// Reject a negative count rather than silently returning an empty list.
		if (0 > count)
			throw new IndexOutOfBoundsException("count cannot be negative: " + count);

		return readArrayImpl(view, factoryClass, comparator, (idx, cursor) -> count > idx);
	}

	/**
	 * Reads array of variable size objects.
	 *
	 * @param <T> Element type.
	 * @param view View positioned at the array, whose window bounds the read.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @param skipLastElementPadding {@code true} if last element is not aligned/padded.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final BufferView view, final Factory<T> factoryClass,
			final int alignment, final boolean skipLastElementPadding) {
		final BufferView cursor = view.snapshot();
		final List<T> elements = new ArrayList<>();
		while (0 < cursor.length()) {
			final T element = factoryClass.deserialize(cursor);
			final int elementSize = element.size();

			if (0 >= elementSize)
				throw new IndexOutOfBoundsException("element size has invalid size");

			elements.add(element);

			final int alignedSize = (skipLastElementPadding && elementSize >= cursor.length())
					? elementSize
					: alignUp(elementSize, alignment);
			if (alignedSize > cursor.length())
				throw new IndexOutOfBoundsException("unexpected buffer length");

			cursor.shiftRight(alignedSize);
		}

		return elements;
	}

	/**
	 * Writes array of objects.
	 *
	 * @param <T> Element type.
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 */
	public static <T extends Serializer> void writeArray(final Writer output, final List<T> elements) {
		writeArray(output, elements, null);
	}

	/**
	 * Writes array of objects, verifying strictly increasing element order.
	 *
	 * @param <T> Element type.
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 * @param comparator Sort-key comparator enforcing element order; {@code null} skips the check.
	 */
	public static <T extends Serializer> void writeArray(final Writer output, final List<T> elements,
			final Comparator<? super T> comparator) {
		writeArrayImpl(output, elements, elements.size(), comparator);
	}

	/**
	 * Writes array of a deterministic number of objects.
	 *
	 * @param <T> Element type.
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 * @param count Number of objects to write.
	 */
	public static <T extends Serializer> void writeArrayCount(final Writer output, final List<T> elements, final int count) {
		writeArrayCount(output, elements, count, null);
	}

	/**
	 * Writes array of a deterministic number of objects, verifying strictly increasing element order.
	 *
	 * @param <T> Element type.
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 * @param count Number of objects to write.
	 * @param comparator Sort-key comparator enforcing element order; {@code null} skips the check.
	 */
	public static <T extends Serializer> void writeArrayCount(final Writer output, final List<T> elements, final int count,
			final Comparator<? super T> comparator) {
		writeArrayImpl(output, elements, count, comparator);
	}

	/**
	 * Writes array of variable size objects.
	 *
	 * @param <T> Element type.
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @param skipLastElementPadding {@code true} if last element should not be aligned/padded.
	 */
	public static <T extends Serializer> void writeVariableSizeElements(final Writer output, final List<T> elements, final int alignment,
			final boolean skipLastElementPadding) {
		for (int index = 0; index < elements.size(); ++index) {
			final T element = elements.get(index);
			output.write(element.serialize());
			if (!skipLastElementPadding || elements.size() - 1 != index) {
				final int elementSize = element.size();
				final int padding = alignUp(elementSize, alignment) - elementSize;
				if (0 != padding)
					output.write(new byte[padding]);
			}
		}
	}

	/**
	 * Writes array of variable size objects without skipping the last element's padding.
	 *
	 * @param <T> Element type.
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 * @param alignment Alignment.
	 */
	public static <T extends Serializer> void writeVariableSizeElements(final Writer output, final List<T> elements, final int alignment) {
		writeVariableSizeElements(output, elements, alignment, false);
	}

	/**
	 * Concatenates byte arrays in order into a single new array.
	 *
	 * @param parts Byte arrays to concatenate, in order.
	 * @return New array holding every part end to end.
	 */
	public static byte[] concat(final byte[]... parts) {
		int total = 0;
		for (final byte[] part : parts)
			total += part.length;

		final byte[] result = new byte[total];
		int offset = 0;
		for (final byte[] part : parts) {
			System.arraycopy(part, 0, result, offset, part.length);
			offset += part.length;
		}

		return result;
	}

	/**
	 * Returns a new array with the bytes of {@code input} in reverse order (NEM reverses the private-key seed before ed25519 use).
	 *
	 * @param input Byte array to reverse.
	 * @return New reversed array.
	 */
	public static byte[] reverse(final byte[] input) {
		final byte[] result = new byte[input.length];
		for (int i = 0; i < input.length; ++i)
			result[i] = input[input.length - 1 - i];

		return result;
	}
}
