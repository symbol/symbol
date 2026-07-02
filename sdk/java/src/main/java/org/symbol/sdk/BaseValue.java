package org.symbol.sdk;

import java.util.Locale;
import java.util.Objects;

import org.symbol.sdk.utils.Converter;

/**
 * Fixed-size integer value (amount, height, mosaic id, …) extended by every generated integer POD; F-bounded for a type-safe
 * {@link Comparable}. The backing {@code long} holds an unsigned u64 {@code >= 2^63} as its negative two's-complement pattern (see
 * {@link #value()}).
 *
 * @param <T> Self-type — the concrete POD extending this class.
 */
public abstract class BaseValue<T extends BaseValue<T>> implements Serializer, Comparable<T> {
	private final long value;

	private final int size;

	private final boolean isSigned;

	/**
	 * Creates a fixed-size integer value, validating that it fits in {@code size} bytes with the given signedness.
	 *
	 * @param value Underlying value as a 64-bit two's-complement bit pattern (unsigned u64 values {@code >= 2^63} are stored negative).
	 * @param size Byte width.
	 * @param isSigned {@code true} if signed.
	 */
	protected BaseValue(final long value, final int size, final boolean isSigned) {
		requireRange(value, size, isSigned);
		this.value = value;
		this.size = size;
		this.isSigned = isSigned;
	}

	/**
	 * Returns the underlying value as a {@code long}. Java {@code long} is signed, so unsigned 64-bit values {@code >= 2^63} are returned
	 * as negative numbers; interpret them with {@link Long#toUnsignedString(long)} and {@link Long#compareUnsigned(long, long)}.
	 *
	 * @return Value (64-bit two's-complement bit pattern).
	 */
	public final long value() {
		return value;
	}

	/**
	 * Returns {@code true} if this value should be interpreted as signed.
	 *
	 * @return {@code true} if signed.
	 */
	public final boolean isSigned() {
		return isSigned;
	}

	@Override
	public final int size() {
		return size;
	}

	@Override
	public final byte[] serialize() {
		return Converter.intToBytes(value, size, isSigned);
	}

	@Override
	public final int compareTo(final T other) {
		return isSigned ? Long.compare(value, other.value()) : Long.compareUnsigned(value, other.value());
	}

	/**
	 * Returns the JSON-safe representation: a number for values below 8 bytes, a base-10 string for 8-byte values (which can exceed the
	 * 2^53 safe-integer range of JSON readers); unsigned 8-byte values render via {@link Long#toUnsignedString(long)}.
	 *
	 * @return JSON-safe representation of this value.
	 */
	public final Object toJson() {
		if (8 <= size)
			return isSigned ? Long.toString(value) : Long.toUnsignedString(value);

		return value;
	}

	@Override
	public final String toString() {
		return toHexString(value, size, isSigned);
	}

	@Override
	public final boolean equals(final Object other) {
		if (this == other)
			return true;

		// match by concrete type as well as width/signedness/value
		if (null == other || getClass() != other.getClass())
			return false;

		final BaseValue<?> rb = (BaseValue<?>) other;
		return size == rb.size && isSigned == rb.isSigned && value == rb.value;
	}

	@Override
	public final int hashCode() {
		return Objects.hash(getClass(), size, isSigned, value);
	}

	// validates that value fits in size bytes with the given signedness; size 8 admits any 64-bit pattern
	private static void requireRange(final long value, final int size, final boolean isSigned) {
		if (1 != size && 2 != size && 4 != size && 8 != size)
			throw new IllegalArgumentException(String.format("\"size\" (%d) must be 1, 2, 4 or 8 bytes", size));

		if (8 == size)
			return;

		final int bits = 8 * size;
		final long lowerBound = isSigned ? -(1L << (bits - 1)) : 0L;
		final long upperBound = (1L << (isSigned ? bits - 1 : bits)) - 1;
		if (value < lowerBound || value > upperBound)
			throw new IllegalArgumentException(String.format("\"value\" (%d) is outside of valid %d-bit range", value, bits));
	}

	/**
	 * Renders {@code value} as a zero-padded uppercase {@code 0x}-prefixed hex string showing the {@code size}-byte unsigned bit pattern
	 * (signed negative values render as their two's-complement).
	 *
	 * @param value Value (64-bit two's-complement bit pattern).
	 * @param size Byte width.
	 * @param isSigned {@code true} if signed (unused; kept for API symmetry).
	 * @return Hex string.
	 */
	static String toHexString(final long value, final int size, final boolean isSigned) {
		final long masked = 8 == size ? value : value & ((1L << (8 * size)) - 1);
		return "0x" + String.format(Locale.ROOT, "%0" + (2 * size) + "X", masked);
	}
}
