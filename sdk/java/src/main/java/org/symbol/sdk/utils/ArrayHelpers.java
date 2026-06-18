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
		 * Deserializes a single element from the given buffer at the specified offset.
		 *
		 * @param buffer Buffer to read from.
		 * @param offset Offset into the buffer at which to start reading.
		 * @return Deserialized element.
		 */
		T deserialize(byte[] buffer, int offset);
	}

	private interface ContinuationPredicate<T extends Serializer> {
		boolean shouldContinue(int index, BufferView view);
	}

	private static <T extends Serializer> List<T> readArrayImpl(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass, final Comparator<? super T> comparator, final ContinuationPredicate<T> shouldContinue) {
		final BufferView view = new BufferView(bufferInput, offset, length);
		final List<T> elements = new ArrayList<>();
		T previousElement = null;
		int i = 0;
		while (shouldContinue.shouldContinue(i, view)) {
			final T element = factoryClass.deserialize(view.backing(), view.offset());

			if (0 >= element.size())
				throw new IndexOutOfBoundsException("element size has invalid size");

			if (null != comparator && null != previousElement && 0 <= comparator.compare(previousElement, element))
				throw new IllegalStateException("elements in array are not sorted");

			elements.add(element);
			view.shiftRight(element.size());
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
	 * Reads array of objects until the buffer window is exhausted, without order verification.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param offset Offset into the buffer at which to start reading.
	 * @param length Length of the readable window.
	 * @param factoryClass Factory used to deserialize objects.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArray(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass) {
		return readArray(bufferInput, offset, length, factoryClass, null);
	}

	/**
	 * Reads array of objects until the buffer window is exhausted, verifying strictly increasing element order.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param offset Offset into the buffer at which to start reading.
	 * @param length Length of the readable window.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param comparator Sort-key comparator enforcing element order; {@code null} skips the check.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArray(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass, final Comparator<? super T> comparator) {
		return readArrayImpl(bufferInput, offset, length, factoryClass, comparator, (idx, view) -> 0 < view.length());
	}

	/**
	 * Reads array of objects from the start of the buffer to its end.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param factoryClass Factory used to deserialize objects.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArray(final byte[] bufferInput, final Factory<T> factoryClass) {
		return readArray(bufferInput, 0, bufferInput.length, factoryClass);
	}

	/**
	 * Reads array of a deterministic number of objects.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param offset Offset into the buffer at which to start reading.
	 * @param length Length of the readable window.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param count Number of objects to deserialize.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArrayCount(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass, final int count) {
		return readArrayCount(bufferInput, offset, length, factoryClass, count, null);
	}

	/**
	 * Reads array of a deterministic number of objects, verifying strictly increasing element order.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param offset Offset into the buffer at which to start reading.
	 * @param length Length of the readable window.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param count Number of objects to deserialize.
	 * @param comparator Sort-key comparator enforcing element order; {@code null} skips the check.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArrayCount(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass, final int count, final Comparator<? super T> comparator) {
		return readArrayImpl(bufferInput, offset, length, factoryClass, comparator, (idx, view) -> count > idx);
	}

	/**
	 * Reads array of deterministic number of objects from the start of the buffer.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param count Number of objects to deserialize.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readArrayCount(final byte[] bufferInput, final Factory<T> factoryClass, final int count) {
		return readArrayCount(bufferInput, 0, bufferInput.length, factoryClass, count);
	}

	/**
	 * Reads array of variable size objects.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param offset Offset into the buffer at which to start reading.
	 * @param length Length of the readable window.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @param skipLastElementPadding {@code true} if last element is not aligned/padded.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass, final int alignment, final boolean skipLastElementPadding) {
		final BufferView view = new BufferView(bufferInput, offset, length);
		final List<T> elements = new ArrayList<>();
		while (0 < view.length()) {
			final T element = factoryClass.deserialize(view.backing(), view.offset());

			if (0 >= element.size())
				throw new IndexOutOfBoundsException("element size has invalid size");

			elements.add(element);

			final int alignedSize = (skipLastElementPadding && element.size() >= view.length())
					? element.size()
					: alignUp(element.size(), alignment);
			if (alignedSize > view.length())
				throw new IndexOutOfBoundsException("unexpected buffer length");

			view.shiftRight(alignedSize);
		}

		return elements;
	}

	/**
	 * Reads array of variable size objects.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @param skipLastElementPadding {@code true} if last element is not aligned/padded.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final byte[] bufferInput, final Factory<T> factoryClass,
			final int alignment, final boolean skipLastElementPadding) {
		return readVariableSizeElements(bufferInput, 0, bufferInput.length, factoryClass, alignment, skipLastElementPadding);
	}

	/**
	 * Reads array of variable size objects at specific offset without skipping the last element's padding.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param offset Offset into the buffer at which to start reading.
	 * @param length Length of the readable window.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param alignment Alignment.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final byte[] bufferInput, final int offset, final int length,
			final Factory<T> factoryClass, final int alignment) {
		return readVariableSizeElements(bufferInput, offset, length, factoryClass, alignment, false);
	}

	/**
	 * Reads array of variable size objects without skipping the last element's padding.
	 *
	 * @param <T> Element type.
	 * @param bufferInput Buffer input.
	 * @param factoryClass Factory used to deserialize objects.
	 * @param alignment Alignment.
	 * @return Array of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final byte[] bufferInput, final Factory<T> factoryClass,
			final int alignment) {
		return readVariableSizeElements(bufferInput, factoryClass, alignment, false);
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
				final int alignedSize = alignUp(element.size(), alignment);
				final int padding = alignedSize - element.size();
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
}
