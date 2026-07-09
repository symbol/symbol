package org.symbol.sdk.utils;

import java.util.Arrays;

/**
 * Base32 encode / decode (RFC 4648, no padding) used by NEM and Symbol addresses.
 */
public final class Base32 {
	private static final char[] ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567".toCharArray();
	private static final int DECODED_BLOCK_SIZE = 5;
	private static final int ENCODED_BLOCK_SIZE = 8;

	// reverse of ALPHABET: maps a char to its 5-bit value, or -1 when the char is not a base32 symbol. Sized to just span
	// the alphabet (its highest char, 'Z', + 1) rather than the full ASCII range; decodeChar bounds-checks above that.
	private static final byte[] CHAR_TO_DECODED;

	static {
		int maxChar = 0;
		for (final char symbol : ALPHABET)
			maxChar = Math.max(maxChar, symbol);

		CHAR_TO_DECODED = new byte[maxChar + 1];
		Arrays.fill(CHAR_TO_DECODED, (byte) -1);
		for (int i = 0; i < ALPHABET.length; ++i)
			CHAR_TO_DECODED[ALPHABET[i]] = (byte) i;
	}

	private Base32() {
	}

	// region encode

	private static int byteToInt(final byte b) {
		return Byte.toUnsignedInt(b);
	}

	private static void encodeBlock(final byte[] input, final int inputOffset, final char[] output, final int outputOffset) {
		output[outputOffset + 0] = ALPHABET[byteToInt(input[inputOffset + 0]) >> 3];
		output[outputOffset + 1] = ALPHABET[((byteToInt(input[inputOffset + 0]) & 0x07) << 2) | (byteToInt(input[inputOffset + 1]) >> 6)];
		output[outputOffset + 2] = ALPHABET[(byteToInt(input[inputOffset + 1]) & 0x3E) >> 1];
		output[outputOffset + 3] = ALPHABET[((byteToInt(input[inputOffset + 1]) & 0x01) << 4) | (byteToInt(input[inputOffset + 2]) >> 4)];
		output[outputOffset + 4] = ALPHABET[((byteToInt(input[inputOffset + 2]) & 0x0F) << 1) | (byteToInt(input[inputOffset + 3]) >> 7)];
		output[outputOffset + 5] = ALPHABET[(byteToInt(input[inputOffset + 3]) & 0x7F) >> 2];
		output[outputOffset + 6] = ALPHABET[((byteToInt(input[inputOffset + 3]) & 0x03) << 3) | (byteToInt(input[inputOffset + 4]) >> 5)];
		output[outputOffset + 7] = ALPHABET[byteToInt(input[inputOffset + 4]) & 0x1F];
	}

	// endregion

	/**
	 * Returns whether {@code c} is a valid base32 (RFC 4648) symbol, i.e. one of {@code A-Z} or {@code 2-7}.
	 *
	 * @param c Character to test.
	 * @return {@code true} if {@code c} is a base32 symbol.
	 */
	public static boolean isValidChar(final char c) {
		return c < CHAR_TO_DECODED.length && 0 <= CHAR_TO_DECODED[c];
	}

	// region decode

	private static byte decodeChar(final char c) {
		final byte decoded = c < CHAR_TO_DECODED.length ? CHAR_TO_DECODED[c] : -1;
		if (0 > decoded)
			throw new IllegalArgumentException(String.format("illegal base32 character %c", c));

		return decoded;
	}

	private static void decodeBlock(final char[] input, final int inputOffset, final byte[] output, final int outputOffset) {
		final byte[] bytes = new byte[ENCODED_BLOCK_SIZE];
		for (int i = 0; i < ENCODED_BLOCK_SIZE; ++i)
			bytes[i] = decodeChar(input[inputOffset + i]);

		output[outputOffset + 0] = (byte) ((bytes[0] << 3) | (bytes[1] >> 2));
		output[outputOffset + 1] = (byte) (((bytes[1] & 0x03) << 6) | (bytes[2] << 1) | (bytes[3] >> 4));
		output[outputOffset + 2] = (byte) (((bytes[3] & 0x0F) << 4) | (bytes[4] >> 1));
		output[outputOffset + 3] = (byte) (((bytes[4] & 0x01) << 7) | (bytes[5] << 2) | (bytes[6] >> 3));
		output[outputOffset + 4] = (byte) (((bytes[6] & 0x07) << 5) | bytes[7]);
	}

	// endregion

	/**
	 * Base32 encodes a binary buffer.
	 *
	 * @param data Binary data to encode.
	 * @return Base32 encoded string corresponding to the input data.
	 */
	public static String encode(final byte[] data) {
		if (0 != data.length % DECODED_BLOCK_SIZE)
			throw new IllegalArgumentException(String.format("decoded size must be multiple of %d", DECODED_BLOCK_SIZE));

		final int outputLength;
		try {
			outputLength = Math.multiplyExact(data.length / DECODED_BLOCK_SIZE, ENCODED_BLOCK_SIZE);
		} catch (final ArithmeticException ex) {
			throw new IllegalArgumentException(String.format("input too large to base32 encode: %d bytes", data.length), ex);
		}

		final char[] output = new char[outputLength];
		for (int i = 0; i < data.length / DECODED_BLOCK_SIZE; ++i)
			encodeBlock(data, i * DECODED_BLOCK_SIZE, output, i * ENCODED_BLOCK_SIZE);

		return new String(output);
	}

	/**
	 * Base32 decodes a base32 encoded string.
	 *
	 * @param encoded Base32 encoded string to decode.
	 * @return Binary data corresponding to the input string.
	 */
	public static byte[] decode(final String encoded) {
		if (0 != encoded.length() % ENCODED_BLOCK_SIZE)
			throw new IllegalArgumentException(String.format("encoded size must be multiple of %d", ENCODED_BLOCK_SIZE));

		final byte[] output = new byte[encoded.length() / ENCODED_BLOCK_SIZE * DECODED_BLOCK_SIZE];
		final char[] encodedChars = encoded.toCharArray();
		for (int i = 0; i < encoded.length() / ENCODED_BLOCK_SIZE; ++i)
			decodeBlock(encodedChars, i * ENCODED_BLOCK_SIZE, output, i * DECODED_BLOCK_SIZE);

		return output;
	}
}
