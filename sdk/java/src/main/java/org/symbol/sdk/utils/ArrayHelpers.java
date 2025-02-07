package org.symbol.sdk.utils;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.function.BiPredicate;
import java.util.function.Function;
import java.util.stream.Stream;

/**
 * Array helper for reading and writing fixed and variable size objects
 */
public class ArrayHelpers {

	/**
	 * Deeply compares two elements.
	 *
	 * @param lhs Left object to compare.
	 * @param rhs Right object to compare.
	 * @return 1 if lhs is greater than rhs; -1 if lhs is less than rhs; 0 if lhs and rhs are equal.
	 */
	public static <U extends Comparable<U>> int deepCompare(final U lhs, final U rhs) {
		return lhs.compareTo(rhs);
	}

	/**
	 * Deeply compares two array elements.
	 *
	 * @param lhs Left object list to compare.
	 * @param rhs Right object list to compare.
	 * @return 1 if lhs is greater than rhs; -1 if lhs is less than rhs; 0 if lhs and rhs are equal.
	 */
	public static <U extends Comparable<U>> int deepCompare(final U[] lhs, final U[] rhs) {
		if (lhs.length != rhs.length)
			return lhs.length > rhs.length ? 1 : -1;

		for (int i = 0; i < lhs.length; ++i) {
			final int compareResult = deepCompare(lhs[i], rhs[i]);
			if (0 != compareResult)
				return compareResult;
		}

		return 0;
	}

	private static <T extends Serializer, U extends Comparable<U>> List<T> readArrayImpl(final ByteBuffer buffer,
			final Function<ByteBuffer, T> factory, final Function<T, U[]> accessor, final BiPredicate<Integer, ByteBuffer> shouldContinue) {
		final List<T> elements = new ArrayList<>();
		T previousElement = null;
		int i = 0;
		while (shouldContinue.test(i, buffer)) {
			final T element = factory.apply(buffer);

			if (0 >= element.getSize())
				throw new IllegalStateException("element size has invalid size");

			if (null != accessor && null != previousElement && 0 <= deepCompare(accessor.apply(previousElement), accessor.apply(element)))
				throw new IllegalStateException("elements in array are not sorted");

			elements.add(element);
			previousElement = element;
			++i;
		}

		return elements;
	}

	private static <T extends Serializer, U extends Comparable<U>> void writeArrayImpl(final Writer output, final List<T> elements,
			final int count, final Function<T, U[]> accessor) {
		for (int i = 0; i < count; ++i) {
			final T element = elements.get(i);
			if (null != accessor && 0 < i && 0 <= deepCompare(accessor.apply(elements.get(i - 1)), accessor.apply(element)))
				throw new IllegalStateException("array passed to write array is not sorted");

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
		return (int) Math.floor((double) (size + alignment - 1) / alignment) * alignment;
	}

	/**
	 * Calculates size of variable size objects.
	 *
	 * @param elements Serializable elements.
	 * @return Computed size.
	 */
	public static int size(final List<Serializer> elements) {
		return size(elements, 0, false);
	}

	/**
	 * Calculates size of variable size objects.
	 *
	 * @param elements Serializable elements.
	 * @param alignment Alignment used for calculations.
	 * @param skipLastElementPadding true if last element should not be aligned.
	 * @return Computed size.
	 */
	public static int size(final List<Serializer> elements, int alignment, boolean skipLastElementPadding) {
		if (elements.isEmpty())
			return 0;

		final Stream<Serializer> stream = elements.stream();
		if (0 == alignment)
			return stream.mapToInt(Serializer::getSize).sum();

		if (!skipLastElementPadding)
			return stream.mapToInt(e -> alignUp(e.getSize(), alignment)).sum();

		return stream.limit(elements.size() - 1).mapToInt(e -> alignUp(e.getSize(), alignment)).sum()
				+ elements.get(elements.size() - 1).getSize();
	}

	/**
	 * Reads array of objects.
	 *
	 * @param buffer Buffer input.
	 * @param factory Factory used to deserialize objects.
	 * @param accessor Optional accessor used to check objects order.
	 * @return List of deserialized objects.
	 */
	public static <U extends Comparable<U>> List<Serializer> readArray(final ByteBuffer buffer,
			final Function<ByteBuffer, Serializer> factory, Function<Serializer, U[]> accessor) {
		// note: this method is used only for '__FILL__' type arrays
		// this loop assumes properly sliced buffer is passed and that there's no additional data.
		return readArrayImpl(buffer, factory, accessor, (__, buf) -> 0 < buf.remaining());
	}

	/**
	 * Reads array of objects.
	 *
	 * @param buffer Buffer input.
	 * @param factory Factory used to deserialize objects.
	 * @return List of deserialized objects.
	 */
	public static List<Serializer> readArray(final ByteBuffer buffer, final Function<ByteBuffer, Serializer> factory) {
		// note: this method is used only for '__FILL__' type arrays
		// this loop assumes properly sliced buffer is passed and that there's no additional data.
		return readArrayImpl(buffer, factory, null, (__, buf) -> 0 < buf.remaining());
	}

	/**
	 * Reads array of deterministic number of objects.
	 *
	 * @param buffer Buffer input.
	 * @param factory Factory used to deserialize objects.
	 * @param count Number of object to deserialize.
	 * @param accessor Optional accessor used to check objects order.
	 * @return List of deserialized objects.
	 */
	public static <U extends Comparable<U>> List<Serializer> readArrayCount(final ByteBuffer buffer,
			final Function<ByteBuffer, Serializer> factory, final int count, Function<Serializer, U[]> accessor) {
		return readArrayImpl(buffer, factory, accessor, (index, __) -> count > index);
	}

	/**
	 * Reads array of deterministic number of objects.
	 *
	 * @param buffer Buffer input.
	 * @param factory Factory used to deserialize objects.
	 * @param count Number of object to deserialize.
	 * @return List of deserialized objects.
	 */
	public static List<Serializer> readArrayCount(final ByteBuffer buffer, final Function<ByteBuffer, Serializer> factory,
			final int count) {
		return readArrayImpl(buffer, factory, null, (index, __) -> count > index);
	}

	/**
	 * Reads array of variable size objects.
	 *
	 * @param buffer Buffer input.
	 * @param factory Factory used to deserialize objects.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @param skipLastElementPadding true if last element is not aligned/padded.
	 * @return List of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final ByteBuffer buffer, final Function<ByteBuffer, T> factory,
			final int alignment, final boolean skipLastElementPadding) {
		final List<T> elements = new ArrayList<>();
		while (0 < buffer.remaining()) {
			final T element = factory.apply(buffer);

			if (0 >= element.getSize())
				throw new IllegalStateException("element size has invalid size");

			elements.add(element);

			final int alignedSize = (skipLastElementPadding && element.getSize() >= buffer.remaining())
					? element.getSize()
					: alignUp(element.getSize(), alignment);
			final int sizeAdjustment = alignedSize - element.getSize();
			if (sizeAdjustment > buffer.remaining())
				throw new IllegalStateException("unexpected buffer length");

			buffer.position(buffer.position() + sizeAdjustment);
		}

		return elements;
	};

	/**
	 * Reads array of variable size objects.
	 *
	 * @param buffer Buffer input.
	 * @param factory Factory used to deserialize objects.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @return List of deserialized objects.
	 */
	public static <T extends Serializer> List<T> readVariableSizeElements(final ByteBuffer buffer, final Function<ByteBuffer, T> factory,
			final int alignment) {
		return readVariableSizeElements(buffer, factory, alignment, false);
	}

	/**
	 * Writes array of objects.
	 *
	 * @param output Output buffer.
	 * @param elements Serializable elements.
	 * @param accessor Optional accessor used to check objects order.
	 */
	public static <U extends Comparable<U>> void writeArray(final Writer output, final List<Serializer> elements,
			final Function<Serializer, U[]> accessor) {
		writeArrayImpl(output, elements, elements.size(), accessor);
	}

	/**
	 * Writes array of objects.
	 *
	 * @param output Output buffer.
	 * @param elements Serializable elements.
	 */
	public static void writeArray(final Writer output, final List<Serializer> elements) {
		writeArrayImpl(output, elements, elements.size(), null);
	}

	/**
	 * Writes array of deterministic number of objects.
	 *
	 * @param output Output buffer.
	 * @param elements Serializable elements.
	 * @param count Number of objects to write.
	 * @param accessor Optional accessor used to check objects order.
	 */
	public static <U extends Comparable<U>> void writeArrayCount(final Writer output, final List<Serializer> elements, final int count,
			Function<Serializer, U[]> accessor) {
		writeArrayImpl(output, elements, count, accessor);
	}

	/**
	 * Writes array of deterministic number of objects.
	 *
	 * @param output Output buffer.
	 * @param elements Serializable elements.
	 * @param count Number of objects to write.
	 */
	public static void writeArrayCount(final Writer output, final List<Serializer> elements, final int count) {
		writeArrayImpl(output, elements, count, null);
	}

	/**
	 * Writes array of variable size objects.
	 *
	 * @param output Output buffer.
	 * @param elements Serializable elements.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 * @param skipLastElementPadding true if last element should not be aligned/padded.
	 */
	public static void writeVariableSizeElements(final Writer output, final List<Serializer> elements, final int alignment,
			boolean skipLastElementPadding) {
		Iterator<Serializer> iterator = elements.iterator();
		while (iterator.hasNext()) {
			final Serializer element = iterator.next();
			output.write(element.serialize());
			if (!skipLastElementPadding || iterator.hasNext()) {
				final int alignedSize = alignUp(element.getSize(), alignment);
				if (0 != (alignedSize - element.getSize()))
					output.write(new byte[alignedSize - element.getSize()]);
			}
		}
	}

	/**
	 * Writes array of variable size objects.
	 *
	 * @param output Output sink.
	 * @param elements Serializable elements.
	 * @param alignment Alignment used to make sure each object is at boundary.
	 */
	public static void writeVariableSizeElements(final Writer output, final List<Serializer> elements, final int alignment) {
		writeVariableSizeElements(output, elements, alignment, false);
	}
}
