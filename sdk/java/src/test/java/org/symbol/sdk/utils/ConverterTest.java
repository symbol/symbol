package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.Test;

public class ConverterTest {

	// toByte tests

	@Test
	public void convertAllValidHexCharCombinationToByte() {
		// Arrange:
		final Map<Character, Integer> charToValueMappings = new HashMap<>();
		for (int code = '0'; code <= '9'; ++code)
			charToValueMappings.put((char) code, code - '0');
		for (int code = 'a'; code <= 'f'; ++code)
			charToValueMappings.put((char) code, code - 'a' + 10);
		for (int code = 'A'; code <= 'F'; ++code)
			charToValueMappings.put((char) code, code - 'A' + 10);

		// Act:
		AtomicInteger numTests = new AtomicInteger();
		charToValueMappings.forEach((key1, code1) -> {
			charToValueMappings.forEach((key2, code2) -> {
				// Act:
				final byte b = Converter.toByte(key1, key2);

				// Assert:
				final byte expected = (byte) ((code1 * 16) + code2);
				assertThat(b, equalTo(expected));
				numTests.incrementAndGet();
			});
		});
	}

	@Test
	public void cannotConvertInvalidHexCharToByte() {
		// Arrange:
		final Map<Character, Character> pairs = new HashMap<>() {
			{
				put('G', '6');
				put('7', 'g');
				put('*', '8');
				put('9', '!');
			}
		};

		// Act:
		pairs.forEach((key, value) -> {
			// Assert:
			final String message = String.format("unrecognized hex char '%c%c'", key, value);
			IllegalArgumentException thrown = assertThrows(IllegalArgumentException.class, () -> Converter.toByte(key, value));
			assertThat(thrown.getMessage(), equalTo(message));
		});
	}

	// isHexString

	@Test
	public void validHexString() {
		// Arrange:
		final String[] inputs = {
				"", "026ee415fc15", "abcdef0123456789ABCDEF"
		};

		// Act:
		Arrays.stream(inputs).forEach(s -> {
			final boolean isHexString = Converter.isHexString(s);

			// Assert:
			assertThat(isHexString, equalTo(true));
		});
	}

	@Test
	public void invalidHexString() {
		// Arrange:
		final String[] inputs = {
				"abcdef012345G789ABCDEF", // invalid ('G') char
				"abcdef0123456789ABCDE" // invalid (odd) length
		};

		// Act:
		Arrays.stream(inputs).forEach(s -> {
			final boolean isHexString = Converter.isHexString(s);

			// Assert:
			assertThat(isHexString, equalTo(false));
		});
	}

	// hexToBytes

	@Test
	public void canParseEmptyHexStringToBytes() {
		// Arrange:
		final String input = "";

		// Act:
		final byte[] actual = Converter.hexToBytes(input);

		// Assert:
		final byte[] expected = new byte[0];
		assertThat(expected, is(equalTo(actual)));
	}

	@Test
	public void canParseHexStringToBytes() {
		// Arrange:
		final String input = "026ee415fc15";

		// Act:
		final byte[] actual = Converter.hexToBytes(input);

		// Assert:
		final byte[] expected = {
				0x02, 0x6E, (byte) 0xE4, 0x15, (byte) 0xFC, 0x15
		};
		assertThat(expected, is(equalTo(actual)));
	}

	@Test
	public void canParseAllValidCharactersHexStringToBytes() {
		// Arrange:
		final String input = "abcdef0123456789ABCDEF";

		// Act:
		final byte[] actual = Converter.hexToBytes(input);

		// Assert:
		final byte[] expected = {
				(byte) 0xAB, (byte) 0xCD, (byte) 0xEF, 0x01, 0x23, 0x45, 0x67, (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF
		};
		assertThat(expected, is(equalTo(actual)));
	}

	@Test
	public void cannotParseInvalidHexStringToBytes() {
		// Arrange:
		final String input = "abcdef012345G789ABCDEF";

		// Act + Assert:
		final String message = String.format("unrecognized hex char 'G7'");
		IllegalArgumentException thrown = assertThrows(IllegalArgumentException.class, () -> Converter.hexToBytes(input));
		assertThat(thrown.getMessage(), equalTo(message));
	}

	@Test
	public void cannotParseInvalidLengthHexStringToBytes() {
		// Arrange:
		final String input = "abcdef0123456789ABCDE";

		// Act + Assert:
		final String message = String.format("hex string has unexpected size '%d'", input.length());
		IllegalArgumentException thrown = assertThrows(IllegalArgumentException.class, () -> Converter.hexToBytes(input));
		assertThat(thrown.getMessage(), equalTo(message));
	}

	// bytesToHex

	@Test
	public void canParseEmptyByteArrayToString() {
		// Arrange:
		final byte[] input = new byte[0];

		// Act:
		final String actual = Converter.bytesToHex(input);

		// Assert:
		final String expected = "";
		assertThat(expected, equalTo(actual));
	}

	@Test
	public void canParseSingleByteArrayToString() {
		// Arrange:
		final byte[] input = {
				(byte) 0xD2
		};

		// Act:
		final String actual = Converter.bytesToHex(input);

		// Assert:
		final String expected = "D2";
		assertThat(expected, equalTo(actual));
	}

	@Test
	public void canParseMultiByteArrayToString() {
		// Arrange:
		final byte[] input = {
				0x02, 0x6E, (byte) 0xE4, 0x15, (byte) 0xFC, 0x15
		};

		// Act:
		final String actual = Converter.bytesToHex(input);

		// Assert:
		final String expected = "026EE415FC15";
		assertThat(expected, equalTo(actual));
	}
}
