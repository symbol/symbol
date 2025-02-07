package org.symbol.sdk.utils;

import java.util.Map;

/**
 * Util class to convert bytes to hex and vice versa.
 */
public class Converter {
	final private static Map<Character, Byte> CHAR_TO_NIBBLE_MAP = CharacterMapBuilder.create(builder -> {
		builder.addRange('0', '9', (byte) 0);
		builder.addRange('a', 'f', (byte) 10);
		builder.addRange('A', 'F', (byte) 10);
	}).build();

	final private static char[] NIBBLE_TO_CHAR_MAP = {
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	private static Byte tryParseByte(final char char1, final char char2) {
		final Byte nibble1 = CHAR_TO_NIBBLE_MAP.get(char1);
		final Byte nibble2 = CHAR_TO_NIBBLE_MAP.get(char2);
		return null == nibble1 || null == nibble2 ? null : (byte) ((nibble1 << 4) | nibble2);
	}

	/**
	 * Decodes two hex characters into a byte.
	 *
	 * @param char1 First hex digit.
	 * @param char2 Second hex digit.
	 * @return Decoded byte.
	 */
	public static Byte toByte(final char char1, final char char2) {
		final Byte decodedByte = tryParseByte(char1, char2);
		if (null == decodedByte)
			throw new IllegalArgumentException(String.format("unrecognized hex char '%c%c'", char1, char2));

		return decodedByte;
	}

	/**
	 * Determines whether a string is a hex string.
	 *
	 * @param input String to test.
	 * @return true if the input is a hex string, false otherwise.
	 */
	public static boolean isHexString(final String input) {
		if (0 != input.length() % 2)
			return false;

		for (int i = 0; i < input.length(); i += 2) {
			if (null == tryParseByte(input.charAt(i), input.charAt(i + 1)))
				return false;
		}

		return true;
	}

	/**
	 * Converts a hex string to a byte array.
	 *
	 * @param input Hex encoded string.
	 * @return array corresponding to the input.
	 */
	public static byte[] hexToBytes(final String input) {
		if (0 != input.length() % 2)
			throw new IllegalArgumentException(String.format("hex string has unexpected size '%d'", input.length()));

		final byte[] output = new byte[input.length() / 2];
		for (int i = 0; i < input.length(); i += 2)
			output[i / 2] = toByte(input.charAt(i), input.charAt(i + 1));

		return output;
	}

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param input Uint8 array.
	 * @return Hex encoded string corresponding to the input.
	 */
	public static String bytesToHex(final byte[] input) {
		final StringBuilder sb = new StringBuilder();
		for (byte b : input) {
			sb.append(NIBBLE_TO_CHAR_MAP[(b >> 4) & 0x0F]);
			sb.append(NIBBLE_TO_CHAR_MAP[b & 0x0F]);
		}

		return sb.toString();
	}
}
