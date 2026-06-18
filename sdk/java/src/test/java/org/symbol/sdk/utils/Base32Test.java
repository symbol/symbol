package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class Base32Test {
	private record TestVector(String decoded, String encoded) {
	}

	private static final TestVector[] TEST_VECTORS = {
			new TestVector("68BA9E8D1AA4502E1F73DA19784B5D7DA16CA1E4AF895FAC12", "NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LAS"),
			new TestVector("684C2605E5B366BB94BC30755EC9F50D74E80FC9283D20E283", "NBGCMBPFWNTLXFF4GB2V5SPVBV2OQD6JFA6SBYUD"),
			new TestVector("68D7B09A14BEA7CE060E71C0FA9AC9B4226DE167013DE10B3D", "NDL3BGQUX2T44BQOOHAPVGWJWQRG3YLHAE66CCZ5"),
			new TestVector("686C44C024F1089669F53C45AC6D62CC17A0D9CBA67A6205E6", "NBWEJQBE6EEJM2PVHRC2Y3LCZQL2BWOLUZ5GEBPG"),
			new TestVector("98A0FE84BBFC5EEE7CADC2B12F790DAA4A7A9505096E674FAB", "TCQP5BF37RPO47FNYKYS66INVJFHVFIFBFXGOT5L")
	};

	@Nested
	final class Encode {
		@Test
		void canConvertEmptyInput() {
			// Act + Assert:
			assertThat(Base32.encode(new byte[0]), equalTo(""));
		}

		@Test
		void canConvertTestVectors() {
			// Act + Assert:
			for (TestVector v : TEST_VECTORS)
				assertThat("input " + v.decoded(), Base32.encode(Converter.hexToUint8(v.decoded())), equalTo(v.encoded()));
		}

		@Test
		void acceptsAllByteValues() {
			// Arrange:
			final byte[] data = new byte[260];
			for (int i = 0; i < 260; ++i)
				data[i] = (byte) (i & 0xFF);

			final String expected = "" + "AAAQEAYEAUDAOCAJBIFQYDIOB4IBCEQTCQKRMFYY" + "DENBWHA5DYPSAIJCEMSCKJRHFAUSUKZMFUXC6MBR"
					+ "GIZTINJWG44DSOR3HQ6T4P2AIFBEGRCFIZDUQSKK" + "JNGE2TSPKBIVEU2UKVLFOWCZLJNVYXK6L5QGCYTD"
					+ "MRSWMZ3INFVGW3DNNZXXA4LSON2HK5TXPB4XU634" + "PV7H7AEBQKBYJBMGQ6EITCULRSGY5D4QSGJJHFEV"
					+ "S2LZRGM2TOOJ3HU7UCQ2FI5EUWTKPKFJVKV2ZLNO" + "V6YLDMVTWS23NN5YXG5LXPF5X274BQOCYPCMLRWH"
					+ "ZDE4VS6MZXHM7UGR2LJ5JVOW27MNTWW33TO55X7A" + "4HROHZHF43T6R2PK5PWO33XP6DY7F47U6X3PP6HZ" + "7L57Z7P674AACAQD";

			// Act + Assert:
			assertThat(Base32.encode(data), equalTo(expected));
		}

		@Test
		void throwsIfInputSizeIsNotMultipleOfBlockSize() {
			// Act + Assert:
			for (int i = 2; i < 10; i += 2) {
				final int size = i;
				final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> Base32.encode(new byte[size]));
				assertThat(ex.getMessage(), containsString("decoded size must be multiple of 5"));
			}
		}
	}

	@Nested
	final class Decode {
		@Test
		void canConvertEmptyInput() {
			// Act + Assert:
			assertThat(Converter.uint8ToHex(Base32.decode("")), equalTo(""));
		}

		@Test
		void canConvertTestVectors() {
			// Act + Assert:
			for (TestVector v : TEST_VECTORS)
				assertThat("input " + v.encoded(), Converter.uint8ToHex(Base32.decode(v.encoded())), equalTo(v.decoded()));
		}

		@Test
		void throwsIfInputSizeIsNotMultipleOfBlockSize() {
			// Act + Assert:
			for (int i = 2; i < 16; i += 2) {
				if (0 == i % 8)
					continue;

				final int size = i;
				final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> Base32.decode("A".repeat(size)));
				assertThat(ex.getMessage(), containsString("encoded size must be multiple of 8"));
			}
		}

		@Test
		void throwsIfInputContainsIllegalChar() {
			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> Base32.decode("AAA1AAAA"));

			// Assert:
			assertThat(ex.getMessage(), containsString("illegal base32 character"));
		}
	}
}
