package org.symbol.sdk;

import java.math.BigInteger;
import java.util.Locale;
import java.util.Objects;

import org.symbol.sdk.utils.Converter;

/**
 * Fixed-size integer value (amount, height, mosaic id, …) extended by every generated integer POD. F-bounded so {@link Comparable} is
 * type-safe per POD. The base class owns the value semantics (equals/hashCode/toString/serialize/compareTo) that an interface cannot
 * provide for {@link Object} methods.
 *
 * @param <T> Self-type — the concrete POD extending this class.
 */
public abstract class BaseValue<T extends BaseValue<T>> implements Serializer, Comparable<T> {
	private final BigInteger value;

	private final int size;

	private final boolean signed;

	/**
	 * Creates a fixed-size integer value, validating that it fits in {@code size} bytes with the given signedness.
	 *
	 * @param value Underlying integer value.
	 * @param size Byte width.
	 * @param isSigned {@code true} if signed.
	 */
	protected BaseValue(final BigInteger value, final int size, final boolean isSigned) {
		requireRange(value, size, isSigned);
		this.value = value;
		this.size = size;
		this.signed = isSigned;
	}

	/**
	 * Returns the underlying integer value.
	 *
	 * @return Value.
	 */
	public final BigInteger value() {
		return value;
	}

	/**
	 * Returns {@code true} if this value should be interpreted as signed.
	 *
	 * @return {@code true} if signed.
	 */
	public final boolean isSigned() {
		return signed;
	}

	@Override
	public final int size() {
		return size;
	}

	@Override
	public final byte[] serialize() {
		return Converter.intToBytes(value, size, signed);
	}

	@Override
	public final int compareTo(final T other) {
		return value.compareTo(other.value());
	}

	/**
	 * Returns the JSON-safe representation: a number for values below 8 bytes, a base-10 string for 8-byte values (which can exceed the
	 * 2^53 safe-integer range of JSON readers).
	 *
	 * @return JSON-safe representation of this value.
	 */
	public final Object toJson() {
		return 8 <= size ? value.toString(10) : (Object) value.longValueExact();
	}

	@Override
	public final String toString() {
		return toHexString(value, size, signed);
	}

	@Override
	public final boolean equals(final Object other) {
		if (this == other)
			return true;

		// match by concrete type as well as width/signedness/value; mirrors the reference SDK's per-type tag so that
		// distinct same-width types (e.g. Amount vs Height, both 8-byte unsigned) never compare equal
		if (null == other || getClass() != other.getClass())
			return false;

		final BaseValue<?> rb = (BaseValue<?>) other;
		return size == rb.size && signed == rb.signed && value.equals(rb.value);
	}

	@Override
	public final int hashCode() {
		return Objects.hash(getClass(), size, signed, value);
	}

	// validates that value fits in size bytes with the given signedness
	private static void requireRange(final BigInteger value, final int size, final boolean isSigned) {
		final int bits = 8 * size;
		final BigInteger lowerBound = isSigned ? BigInteger.ONE.shiftLeft(bits - 1).negate() : BigInteger.ZERO;
		final BigInteger upperBound = BigInteger.ONE.shiftLeft(isSigned ? bits - 1 : bits).subtract(BigInteger.ONE);

		if (0 > value.compareTo(lowerBound) || 0 < value.compareTo(upperBound))
			throw new IllegalArgumentException(String.format("\"value\" (%s) is outside of valid %d-bit range", value, bits));
	}

	/**
	 * Renders {@code value} as a zero-padded uppercase {@code 0x}-prefixed hex string; signed negative values are rendered as unsigned
	 * two's-complement.
	 *
	 * @param value Value to render.
	 * @param size Byte width.
	 * @param isSigned {@code true} if signed.
	 * @return Hex string.
	 */
	public static String toHexString(final BigInteger value, final int size, final boolean isSigned) {
		final BigInteger unsigned = !isSigned || 0 <= value.signum() ? value : value.add(BigInteger.ONE.shiftLeft(size * 8));
		return "0x" + String.format(Locale.ROOT, "%0" + (size * 2) + "X", unsigned);
	}

	/**
	 * Convert a descriptor value (integral {@link Number}, {@link BigInteger}, or {@link String}) to a {@link BigInteger}. A string is
	 * decimal by default, or hexadecimal when {@code 0x}/{@code 0X}-prefixed (so ids conventionally written in hex — mosaic id, namespace
	 * id — parse naturally).
	 *
	 * @param value Raw descriptor value.
	 * @return Converted value.
	 */
	public static BigInteger toBigInteger(final Object value) {
		if (value instanceof BigInteger bigInteger)
			return bigInteger;

		if (value instanceof Integer || value instanceof Long || value instanceof Short || value instanceof Byte)
			return BigInteger.valueOf(((Number) value).longValue());

		if (value instanceof String string) {
			try {
				if (string.startsWith("0x") || string.startsWith("0X"))
					return new BigInteger(string.substring(2), 16);

				return new BigInteger(string);
			} catch (final NumberFormatException ex) {
				throw new InvalidDescriptorException("cannot parse \"" + string + "\" as an integer value", ex);
			}
		}

		throw new InvalidDescriptorException(
				"cannot convert " + (null == value ? "null" : value.getClass().getName()) + " to an integer value");
	}

	/**
	 * Convert a descriptor value to an exact {@code int} via {@link #toBigInteger(Object)}; out-of-range values are rejected rather than
	 * silently truncated.
	 *
	 * @param value Raw descriptor value.
	 * @return Converted value.
	 */
	public static int toInt(final Object value) {
		try {
			return toBigInteger(value).intValueExact();
		} catch (final ArithmeticException ex) {
			throw new InvalidDescriptorException("value " + value + " does not fit in an int", ex);
		}
	}

	/**
	 * Convert a descriptor value to an exact {@code long} via {@link #toBigInteger(Object)}; out-of-range values are rejected rather than
	 * silently truncated.
	 *
	 * @param value Raw descriptor value.
	 * @return Converted value.
	 */
	public static long toLong(final Object value) {
		try {
			return toBigInteger(value).longValueExact();
		} catch (final ArithmeticException ex) {
			throw new InvalidDescriptorException("value " + value + " does not fit in a long", ex);
		}
	}
}
