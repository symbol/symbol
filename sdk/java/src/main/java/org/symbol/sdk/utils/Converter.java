package org.symbol.sdk.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HexFormat;

/**
 * Hex / integer / byte conversion helpers.
 */
public final class Converter {
	private static final HexFormat HEX_UPPER = HexFormat.of().withUpperCase();
	private static final HexFormat HEX_PARSER = HexFormat.of();

	private Converter() {
	}

	/**
	 * Determines whether a string is a hex string.
	 *
	 * @param input String to test.
	 * @return {@code true} if the input is a hex string, {@code false} otherwise.
	 */
	public static boolean isHexString(final String input) {
		if (null == input || 0 != input.length() % 2)
			return false;

		return input.chars().allMatch(HexFormat::isHexDigit);
	}

	/**
	 * Converts a hex string to a byte array.
	 *
	 * @param input Hex encoded string.
	 * @return Byte array corresponding to the input.
	 */
	public static byte[] hexToUint8(final String input) {
		if (0 != input.length() % 2)
			throw new IllegalArgumentException(String.format("hex string has unexpected size '%d'", input.length()));

		try {
			return HEX_PARSER.parseHex(input);
		} catch (IllegalArgumentException ex) {
			throw new IllegalArgumentException(String.format("unrecognized hex char in '%s'", input), ex);
		}
	}

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param input Byte array.
	 * @return Hex encoded string corresponding to the input.
	 */
	public static String uint8ToHex(final byte[] input) {
		return HEX_UPPER.formatHex(input);
	}

	static void validateByteSize(final int size) {
		if (1 != size && 2 != size && 4 != size && 8 != size)
			throw new IllegalArgumentException(String.format("unsupported int size %d", size));
	}

	/**
	 * Converts little-endian bytes to a {@code long} for any size (1, 2, 4 or 8); a size-8 u64 {@code >= 2^63} reads back negative.
	 *
	 * @param input Byte array.
	 * @param size Number of bytes (1, 2, 4 or 8).
	 * @param isSigned {@code true} if a value narrower than 8 bytes should be sign-extended.
	 * @return Value corresponding to the input.
	 */
	public static long bytesToInt(final byte[] input, final int size, final boolean isSigned) {
		return bytesToInt(input, 0, size, isSigned);
	}

	/**
	 * Converts little-endian bytes to a {@code long} for any size (1, 2, 4 or 8); a size-8 u64 {@code >= 2^63} reads back negative.
	 *
	 * @param input Byte array.
	 * @param offset Offset into the array at which to start reading.
	 * @param size Number of bytes (1, 2, 4 or 8).
	 * @param isSigned {@code true} if a value narrower than 8 bytes should be sign-extended.
	 * @return Value corresponding to the input.
	 */
	public static long bytesToInt(final byte[] input, final int offset, final int size, final boolean isSigned) {
		validateByteSize(size);
		final ByteBuffer buffer = ByteBuffer.wrap(input, offset, size).order(ByteOrder.LITTLE_ENDIAN);
		return readInt(buffer, size, isSigned);
	}

	/**
	 * Reads a little-endian integer of {@code size} bytes at {@code buffer}'s current position without advancing it, sign- or zero-extended
	 * into a {@code long} (an 8-byte value fills it exactly). Shared by {@link #bytesToInt} and {@code BufferView.peekInt}.
	 *
	 * @param buffer Little-endian buffer to read from, positioned at the value to read.
	 * @param size Byte width (1, 2, 4 or 8).
	 * @param isSigned {@code true} to sign-extend a narrower width (ignored for {@code size == 8}).
	 * @return Decoded value.
	 */
	static long readInt(final ByteBuffer buffer, final int size, final boolean isSigned) {
		final int position = buffer.position();
		return switch (size) {
			case 1 -> isSigned ? buffer.get(position) : Byte.toUnsignedLong(buffer.get(position));
			case 2 -> isSigned ? buffer.getShort(position) : Short.toUnsignedLong(buffer.getShort(position));
			case 4 -> isSigned ? buffer.getInt(position) : Integer.toUnsignedLong(buffer.getInt(position));
			case 8 -> buffer.getLong(position);
			default -> throw new IllegalArgumentException(String.format("unsupported int size %d", size));
		};
	}

	/**
	 * Converts little-endian bytes to a {@code long} for any size (1, 2, 4 or 8); a size-8 u64 {@code >= 2^63} reads back negative.
	 *
	 * @param input Byte array.
	 * @param size Number of bytes (1, 2, 4 or 8).
	 * @return Unsigned value.
	 */
	public static long bytesToInt(final byte[] input, final int size) {
		return bytesToInt(input, size, false);
	}

	/**
	 * Converts an integer to little-endian bytes by truncating its 64-bit two's-complement encoding to {@code byteSize}.
	 *
	 * @param value Integer value.
	 * @param byteSize Number of output bytes.
	 * @param isSigned {@code true} if the value is signed (unused; kept for API symmetry).
	 * @return Byte representation of the integer.
	 */
	public static byte[] intToBytes(final long value, final int byteSize, final boolean isSigned) {
		validateByteSize(byteSize);

		// extract the low byteSize little-endian bytes directly
		final byte[] result = new byte[byteSize];
		for (int i = 0; i < byteSize; ++i)
			result[i] = (byte) (value >>> (8 * i));

		return result;
	}

	/**
	 * Converts an unsigned integer to little-endian bytes by truncating its 64-bit two's-complement encoding to {@code byteSize}.
	 *
	 * @param value Integer value.
	 * @param byteSize Number of output bytes.
	 * @return Byte representation of the integer.
	 */
	public static byte[] intToBytes(final long value, final int byteSize) {
		return intToBytes(value, byteSize, false);
	}

	/**
	 * Parses a string to a 64-bit {@code long} bit pattern: a {@code 0x}/{@code 0X}-prefixed hex string or a non-negative decimal is read
	 * across the full unsigned range (a value {@code >= 2^63} yields the negative two's-complement pattern), while a {@code -}-prefixed
	 * decimal is parsed signed — so, like {@link #toLong(Number)}, negatives are accepted.
	 *
	 * @param value String value (signed/unsigned decimal, or {@code 0x}-prefixed hexadecimal).
	 * @return Parsed value as a 64-bit two's-complement bit pattern.
	 * @throws IllegalArgumentException If the string is not a valid integer that fits a 64-bit pattern.
	 */
	public static long toLong(final String value) {
		try {
			if (value.startsWith("0x") || value.startsWith("0X"))
				return Long.parseUnsignedLong(value.substring(2), 16);

			// a leading '-' needs signed parsing; without it the decimal is non-negative and safe to read across the full unsigned range
			if (value.startsWith("-"))
				return Long.parseLong(value);

			return Long.parseUnsignedLong(value);
		} catch (final NumberFormatException ex) {
			throw new IllegalArgumentException("cannot parse \"" + value + "\" as an integer value", ex);
		}
	}

	/**
	 * Converts a fixed-width integral wrapper ({@link Integer}/{@link Long}/{@link Short}/{@link Byte}) to a 64-bit {@code long}; any other
	 * type is rejected.
	 *
	 * @param value Integral descriptor number.
	 * @return Converted value as a 64-bit two's-complement bit pattern.
	 * @throws IllegalArgumentException If the value is not one of the fixed-width integral wrappers.
	 */
	public static long toLong(final Number value) {
		if (value instanceof Integer || value instanceof Long || value instanceof Short || value instanceof Byte)
			return value.longValue();

		throw new IllegalArgumentException("cannot convert " + (null == value ? "null" : value.getClass().getName()) + " to a long value");
	}

	/**
	 * Converts a fixed-width integral wrapper to a 32-bit {@code int} bit pattern, accepting the signed {@code int} range and the full
	 * unsigned 32-bit range (a magnitude {@code >= 2^31} yields the negative two's-complement pattern). Anything outside
	 * {@code [Integer.MIN_VALUE, 2^32 - 1]} is rejected rather than truncated.
	 *
	 * @param value Integral descriptor number.
	 * @return Converted value as a 32-bit two's-complement bit pattern.
	 * @throws IllegalArgumentException If the value does not fit a 32-bit pattern (or is not an integral wrapper).
	 */
	public static int toInt(final Number value) {
		final long longValue = toLong(value);
		if (longValue < Integer.MIN_VALUE || longValue > 0xFFFFFFFFL)
			throw new IllegalArgumentException("cannot convert " + value.getClass().getName() + " to a int value");

		return (int) longValue;
	}

	/**
	 * Parses a string to a 32-bit {@code int} bit pattern, the 32-bit analogue of {@link #toLong(String)}: a {@code 0x}-prefixed hex string
	 * or a non-negative decimal is read across the full unsigned 32-bit range (a value {@code >= 2^31} yields the negative two's-complement
	 * pattern), while a {@code -}-prefixed decimal is parsed signed. Anything outside the unsigned 32-bit range is rejected rather than
	 * truncated.
	 *
	 * @param value String value (signed/unsigned decimal, or {@code 0x}-prefixed hexadecimal).
	 * @return Parsed value as a 32-bit two's-complement bit pattern.
	 * @throws IllegalArgumentException If the string is unparseable or does not fit a 32-bit pattern.
	 */
	public static int toInt(final String value) {
		try {
			if (value.startsWith("0x") || value.startsWith("0X"))
				return Integer.parseUnsignedInt(value.substring(2), 16);

			// a leading '-' needs signed parsing; without it the decimal is non-negative and read across the full unsigned range
			if (value.startsWith("-"))
				return Integer.parseInt(value);

			return Integer.parseUnsignedInt(value);
		} catch (final NumberFormatException ex) {
			throw new IllegalArgumentException("value \"" + value + "\" does not fit in an int", ex);
		}
	}
}
