package org.symbol.sdk.utils;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
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

	private static void validateByteSize(final int size) {
		if (1 != size && 2 != size && 4 != size && 8 != size)
			throw new IllegalArgumentException(String.format("unsupported int size %d", size));
	}

	/**
	 * Converts bytes to a little-endian number.
	 *
	 * @param input Byte array.
	 * @param size Number of bytes (1, 2 or 4).
	 * @param isSigned {@code true} if number should be treated as signed.
	 * @return Value corresponding to the input.
	 */
	public static long bytesToInt(final byte[] input, final int size, final boolean isSigned) {
		return bytesToInt(input, 0, size, isSigned);
	}

	/**
	 * Converts bytes to a little-endian number, reading from a given offset.
	 *
	 * @param input Byte array.
	 * @param offset Offset into the array at which to start reading.
	 * @param size Number of bytes (1, 2 or 4).
	 * @param isSigned {@code true} if number should be treated as signed.
	 * @return Value corresponding to the input.
	 */
	public static long bytesToInt(final byte[] input, final int offset, final int size, final boolean isSigned) {
		if (1 != size && 2 != size && 4 != size)
			throw new IllegalArgumentException(String.format("unsupported int size %d", size));

		return readInt(input, offset, size, isSigned);
	}

	/**
	 * Convenience overload for unsigned reads.
	 *
	 * @param input Byte array.
	 * @param size Number of bytes.
	 * @return Unsigned value.
	 */
	public static long bytesToInt(final byte[] input, final int size) {
		return bytesToInt(input, size, false);
	}

	/**
	 * Converts bytes to a little-endian {@link BigInteger}.
	 *
	 * @param input Byte array.
	 * @param size Number of bytes (must be 8).
	 * @param isSigned {@code true} if number should be treated as signed.
	 * @return Value corresponding to the input.
	 */
	public static BigInteger bytesToBigInt(final byte[] input, final int size, final boolean isSigned) {
		return bytesToBigInt(input, 0, size, isSigned);
	}

	/**
	 * Converts bytes to a little-endian {@link BigInteger}, reading from a given offset.
	 *
	 * @param input Byte array.
	 * @param offset Offset into the array at which to start reading.
	 * @param size Number of bytes (must be 8).
	 * @param isSigned {@code true} if number should be treated as signed.
	 * @return Value corresponding to the input.
	 */
	public static BigInteger bytesToBigInt(final byte[] input, final int offset, final int size, final boolean isSigned) {
		if (8 != size)
			throw new IllegalArgumentException(String.format("unsupported int size %d", size));

		return readBigInt(input, offset, 8, isSigned);
	}

	/**
	 * Convenience overload for unsigned reads.
	 *
	 * @param input Byte array.
	 * @param size Number of bytes.
	 * @return Unsigned value.
	 */
	public static BigInteger bytesToBigInt(final byte[] input, final int size) {
		return bytesToBigInt(input, size, false);
	}

	/**
	 * Converts an integer to bytes by truncating its 64-bit little-endian two's-complement encoding to {@code byteSize}.
	 *
	 * @param value Integer value.
	 * @param byteSize Number of output bytes.
	 * @param isSigned {@code true} if the value is signed (unused; kept for API symmetry).
	 * @return Byte representation of the integer.
	 */
	public static byte[] intToBytes(final long value, final int byteSize, final boolean isSigned) {
		validateByteSize(byteSize);
		final byte[] eight = ByteBuffer.allocate(8).order(ByteOrder.LITTLE_ENDIAN).putLong(value).array();
		return 8 == byteSize ? eight : Arrays.copyOf(eight, byteSize);
	}

	/**
	 * Convenience overload for unsigned writes.
	 *
	 * @param value Integer value.
	 * @param byteSize Number of output bytes.
	 * @return Byte representation of the integer.
	 */
	public static byte[] intToBytes(final long value, final int byteSize) {
		return intToBytes(value, byteSize, false);
	}

	/**
	 * Converts a {@link BigInteger} to bytes (little-endian); {@link BigInteger#longValue()} returns the low 64 bits in two's complement,
	 * so delegating to the {@code long} overload produces the same truncated bytes.
	 *
	 * @param value Integer value.
	 * @param byteSize Number of output bytes.
	 * @param isSigned {@code true} if the value is signed.
	 * @return Byte representation of the integer.
	 */
	public static byte[] intToBytes(final BigInteger value, final int byteSize, final boolean isSigned) {
		validateByteSize(byteSize);
		return intToBytes(value.longValue(), byteSize, isSigned);
	}

	/**
	 * Convenience overload for unsigned writes.
	 *
	 * @param value Integer value.
	 * @param byteSize Number of output bytes.
	 * @return Byte representation of the integer.
	 */
	public static byte[] intToBytes(final BigInteger value, final int byteSize) {
		return intToBytes(value, byteSize, false);
	}

	private static long readInt(final byte[] input, final int offset, final int size, final boolean isSigned) {
		ByteBuffer buffer = ByteBuffer.wrap(input, offset, size).order(ByteOrder.LITTLE_ENDIAN);

		return switch (size) {
			case 1 -> isSigned ? buffer.get() : Byte.toUnsignedLong(buffer.get());
			case 2 -> isSigned ? buffer.getShort() : Short.toUnsignedInt(buffer.getShort());
			case 4 -> isSigned ? buffer.getInt() : Integer.toUnsignedLong(buffer.getInt());
			default -> throw new IllegalArgumentException(String.format("unsupported int size %d", size));
		};
	}

	private static BigInteger readBigInt(final byte[] input, final int offset, final int size, final boolean isSigned) {
		final long bits = ByteBuffer.wrap(input, offset, size).order(ByteOrder.LITTLE_ENDIAN).getLong();
		BigInteger value = BigInteger.valueOf(bits);
		// getLong reads bits as signed two's complement; for unsigned interpretation, re-add 2^64
		// whenever the high bit was set (so getLong returned a negative).
		if (!isSigned && bits < 0)
			value = value.add(BigInteger.ONE.shiftLeft(64));

		return value;
	}
}
