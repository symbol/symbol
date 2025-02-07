package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import org.junit.Test;

public class Base32Test {
	final Map<String, String> testVectors = new HashMap<>() {
		{
			put("68BA9E8D1AA4502E1F73DA19784B5D7DA16CA1E4AF895FAC12", "NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LAS");
			put("684C2605E5B366BB94BC30755EC9F50D74E80FC9283D20E283", "NBGCMBPFWNTLXFF4GB2V5SPVBV2OQD6JFA6SBYUD");
			put("68D7B09A14BEA7CE060E71C0FA9AC9B4226DE167013DE10B3D", "NDL3BGQUX2T44BQOOHAPVGWJWQRG3YLHAE66CCZ5");
			put("686C44C024F1089669F53C45AC6D62CC17A0D9CBA67A6205E6", "NBWEJQBE6EEJM2PVHRC2Y3LCZQL2BWOLUZ5GEBPG");
			put("98A0FE84BBFC5EEE7CADC2B12F790DAA4A7A9505096E674FAB", "TCQP5BF37RPO47FNYKYS66INVJFHVFIFBFXGOT5L");
		}
	};

	// encode

	@Test
	public void canEncodeEmptyByteArray() {
		// Arrange:
		final byte[] input = {};

		// Act:
		final String encoded = Base32.encode(input);

		// Assert:
		final String expected = "";
		assertThat(expected, equalTo(encoded));
	}

	@Test
	public void canEncodeTestVectors() {
		// Arrange:
		testVectors.forEach((decoded, encoded) -> {
			final byte[] input = Converter.hexToBytes(decoded);

			// Act:
			final String actual = Base32.encode(input);

			// Assert:
			assertThat(actual, equalTo(encoded));
		});
	}

	@Test
	public void canEncodeAllByteValues() {
		// Arrange:
		final byte[] data = new byte[260];
		for (int i = 0; 260 > i; ++i)
			data[i] = (byte) (i & 0xFF);

		// Act:
		final String encoded = Base32.encode(data);

		// Assert:
		final String expected = "AAAQEAYEAUDAOCAJBIFQYDIOB4IBCEQTCQKRMFYY" + "DENBWHA5DYPSAIJCEMSCKJRHFAUSUKZMFUXC6MBR"
				+ "GIZTINJWG44DSOR3HQ6T4P2AIFBEGRCFIZDUQSKK" + "JNGE2TSPKBIVEU2UKVLFOWCZLJNVYXK6L5QGCYTD"
				+ "MRSWMZ3INFVGW3DNNZXXA4LSON2HK5TXPB4XU634" + "PV7H7AEBQKBYJBMGQ6EITCULRSGY5D4QSGJJHFEV"
				+ "S2LZRGM2TOOJ3HU7UCQ2FI5EUWTKPKFJVKV2ZLNO" + "V6YLDMVTWS23NN5YXG5LXPF5X274BQOCYPCMLRWH"
				+ "ZDE4VS6MZXHM7UGR2LJ5JVOW27MNTWW33TO55X7A" + "4HROHZHF43T6R2PK5PWO33XP6DY7F47U6X3PP6HZ" + "7L57Z7P674AACAQD";
		assertThat(expected, equalTo(encoded));
	}

	@Test
	public void encodeThrowsWhenInputSizeIsIncorrect() {
		// Arrange:
		for (int i = 2; 10 > i; i += 2) {
			final byte[] input = new byte[i];

			// Act + Assert:
			final String message = String.format("decoded size must be multiple of 5");
			IllegalArgumentException thrown = assertThrows(IllegalArgumentException.class, () -> Base32.encode(input));
			assertThat(thrown.getMessage(), equalTo(message));
		}
	}

	// decode

	@Test
	public void canDecodeEmptyString() {
		// Arrange:
		final String input = "";

		// Act:
		final byte[] decoded = Base32.decode(input);

		// Assert:
		final byte[] expected = new byte[0];
		assertThat(expected, equalTo(decoded));
	}

	@Test
	public void canDecodeTestVectors() {
		// Arrange:
		testVectors.forEach((decoded, encoded) -> {
			// Act:
			final byte[] actual = Base32.decode(encoded);

			// Assert:
			assertThat(Converter.bytesToHex(actual), equalTo(decoded));
		});
	}

	@Test
	public void canDecodeAllValidCharacters() {
		// Act:
		final byte[] decoded = Base32.decode("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");

		// Assert:
		assertThat(Converter.bytesToHex(decoded), equalTo("00443214C74254B635CF84653A56D7C675BE77DF"));
	}

	@Test
	public void decodeThrowsWhenInputSizeIsIncorrect() {
		// Arrange:
		for (int i = 1; 8 > i; ++i) {
			final String input = "A".repeat(i);

			// Act + Assert:
			final String message = String.format("encoded size must be multiple of 8");
			IllegalArgumentException thrown = assertThrows(IllegalArgumentException.class, () -> Base32.decode(input));
			assertThat(thrown.getMessage(), equalTo(message));
		}
	}

	@Test
	public void decodeThrowsWhenInputHasInvalidCharacters() {
		// Arrange:
		final Map<String, Character> illegalInputs = new HashMap<>() {
			{
				put("NC5J5DI2URIC4H3T3IMXQS21PWQWZIPEV6EV7LAS", '1'); // contains char '1'
				put("NBGCMBPFWNTLXFF4GB2V5SPV!V2OQD6JFA6SBYUD", '!'); // contains char '!'
				put("NDL3BGQUX2T44BQOOHAPVGWJWQRG3YLHAE)6CCZ5", ')'); // contains char ')'
			}
		};

		// Act + Assert:
		illegalInputs.forEach((input, invalidChar) -> {
			IllegalArgumentException thrown = assertThrows(IllegalArgumentException.class, () -> Base32.decode(input));
			assertThat(thrown.getMessage(), equalTo(String.format("illegal base32 character '%c'", invalidChar)));
		});
	}

	// round trip

	@Test
	public void canDecodeThenEncodeValue() {
		// Arrange: inputs
		final String[] inputs = {
				"BDS73DQ5NC33MKYI3K6GXLJ53C2HJ35A", "46FNYP7T4DD3SWAO6C4NX62FJI5CBA26"
		};
		Arrays.stream(inputs).forEach(input -> {
			// Act:
			final byte[] decoded = Base32.decode(input);
			final String result = Base32.encode(decoded);

			// Assert:
			assertThat(input, equalTo(result));
		});
	}

	@Test
	public void canEncodeThenDecodeValue() {
		// Arrange: inputs
		final String[] inputs = {
				"8A4E7DF5B61CC0F97ED572A95F6ACA", "2D96E4ABB65F0AD3C29FEA48C132CE"
		};
		Arrays.stream(inputs).forEach(input -> {
			// Act:
			final String encoded = Base32.encode(Converter.hexToBytes(input));
			final byte[] result = Base32.decode(encoded);

			// Assert:
			assertThat(input, equalTo(Converter.bytesToHex(result)));
		});
	}
}
