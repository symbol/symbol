package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.math.BigInteger;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class ConverterTest {
	@Nested
	final class IsHexString {
		@Test
		void rejectsNonAsciiUnicodeDigits() {
			// fullwidth and Arabic-Indic digits are Unicode digits but not hex
			// Act + Assert:
			assertThat(Converter.isHexString("\uFF15\uFF15"), is(false));
			assertThat(Converter.isHexString("\u0665\u0665"), is(false));
		}

		@Test
		void returnsTrueForValidHexStrings() {
			// Arrange:
			final String[] inputs = {
					"", "026ee415fc15", "abcdef0123456789ABCDEF"
			};

			// Act + Assert:
			for (String input : inputs)
				assertThat("input " + input, Converter.isHexString(input), is(true));
		}

		@Test
		void returnsFalseForInvalidHexStrings() {
			// Arrange:
			final String[] inputs = {
					"abcdef012345G789ABCDEF", // invalid ('G') char
					"abcdef0123456789ABCDE" // invalid (odd) length
			};

			// Act + Assert:
			for (String input : inputs)
				assertThat("input " + input, Converter.isHexString(input), is(false));
		}

		@Test
		void acceptsEmptyString() {
			// Act + Assert:
			assertThat(Converter.isHexString(""), is(true));
		}

		@Test
		void acceptsSingleOctet() {
			// Act + Assert:
			assertThat(Converter.isHexString("FF"), is(true));
			assertThat(Converter.isHexString("00"), is(true));
		}

		@Test
		void rejectsSingleCharacter() {
			// Act + Assert:
			assertThat(Converter.isHexString("F"), is(false));
		}

		@Test
		void acceptsMixedCase() {
			// Act + Assert:
			assertThat(Converter.isHexString("AbCdEf0123"), is(true));
		}
	}

	@Nested
	final class HexToUint8 {
		@Test
		void canParseValidHexString() {
			// Act:
			final byte[] result = Converter.hexToUint8("026ee415fc15");

			// Assert:
			assertThat(result, equalTo(new byte[]{
					0x02, 0x6e, (byte) 0xe4, 0x15, (byte) 0xfc, 0x15
			}));
		}

		@Test
		void cannotParseHexStringWithOddLength() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.hexToUint8("026ee415fc1"));
		}

		@Test
		void cannotParseHexStringWithInvalidChar() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.hexToUint8("026Ge415fc15"));
		}

		@Test
		void canParseEmptyString() {
			// Act:
			final byte[] result = Converter.hexToUint8("");

			// Assert:
			assertThat(result, equalTo(new byte[0]));
		}

		@Test
		void canParseSingleOctet() {
			// Act:
			final byte[] result = Converter.hexToUint8("FF");

			// Assert:
			assertThat(result, equalTo(new byte[]{
					(byte) 0xFF
			}));
		}

		@Test
		void canParseMixedCase() {
			// Act:
			final byte[] result = Converter.hexToUint8("AbCdEf");

			// Assert:
			assertThat(result, equalTo(new byte[]{
					(byte) 0xAB, (byte) 0xCD, (byte) 0xEF
			}));
		}
	}

	@Nested
	final class Uint8ToHex {
		@Test
		void canEncodeArbitraryBytes() {
			// Act:
			final String result = Converter.uint8ToHex(new byte[]{
					0x02, 0x6e, (byte) 0xe4, 0x15, (byte) 0xfc, 0x15
			});

			// Assert:
			assertThat(result, equalTo("026EE415FC15"));
		}

		@Test
		void canEncodeEmptyArray() {
			// Act + Assert:
			assertThat(Converter.uint8ToHex(new byte[0]), equalTo(""));
		}

		@Test
		void canEncodeSingleByte() {
			// Act + Assert:
			assertThat(Converter.uint8ToHex(new byte[]{
					(byte) 0xFF
			}), equalTo("FF"));
		}

		@Test
		void canEncodeZeroBytes() {
			// Act + Assert:
			assertThat(Converter.uint8ToHex(new byte[]{
					0x00, 0x00, 0x00
			}), equalTo("000000"));
		}

		@Test
		void outputsUppercaseHex() {
			// Act:
			final String result = Converter.uint8ToHex(new byte[]{
					(byte) 0xAB, (byte) 0xCD, (byte) 0xEF
			});

			// Assert:
			assertThat(result, equalTo("ABCDEF"));
		}
	}

	@Nested
	final class BytesToInt {
		@Test
		void readsUint8() {
			// Act + Assert:
			assertThat(Converter.bytesToInt(new byte[]{
					(byte) 0xCD
			}, 1), equalTo(0xCDL));
		}

		@Test
		void readsUint16() {
			// Act + Assert:
			assertThat(Converter.bytesToInt(new byte[]{
					(byte) 0xCD, (byte) 0xAB
			}, 2), equalTo(0xABCDL));
		}

		@Test
		void readsUint32() {
			// Act + Assert:
			assertThat(Converter.bytesToInt(new byte[]{
					0x12, 0x34, 0x56, 0x78
			}, 4), equalTo(0x78563412L));
		}

		@Test
		void readsInt8() {
			// Act + Assert:
			assertThat(Converter.bytesToInt(new byte[]{
					(byte) 0xCD
			}, 1, true), equalTo(-51L));
		}

		@Test
		void readsInt16() {
			// Arrange: 0xFF 0xFF -> -1 in little-endian signed
			final byte[] bytes = {
					(byte) 0xFF, (byte) 0xFF
			};

			// Act + Assert:
			assertThat(Converter.bytesToInt(bytes, 2, true), equalTo(-1L));
		}

		@Test
		void readsInt16Positive() {
			// Arrange: 0x00 0x80 -> 0x8000 (32768) unsigned, but -32768 signed
			final byte[] bytes = {
					0x00, (byte) 0x80
			};

			// Act + Assert:
			assertThat(Converter.bytesToInt(bytes, 2, true), equalTo(-32768L));
		}

		@Test
		void readsInt32() {
			// Arrange: 0xFF 0xFF 0xFF 0xFF -> -1 in little-endian signed
			final byte[] bytes = {
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			};

			// Act + Assert:
			assertThat(Converter.bytesToInt(bytes, 4, true), equalTo(-1L));
		}

		@Test
		void readsInt32Positive() {
			// Arrange: 0x00 0x00 0x00 0x80 -> 0x80000000 unsigned (-2147483648 signed)
			final byte[] bytes = {
					0x00, 0x00, 0x00, (byte) 0x80
			};

			// Act + Assert:
			assertThat(Converter.bytesToInt(bytes, 4, true), equalTo(-2147483648L));
		}

		@Test
		void readsFromOffsetUint8() {
			// Arrange:
			final byte[] bytes = {
					0x11, 0x22, (byte) 0xCD
			};

			// Act:
			final long value = Converter.bytesToInt(bytes, 2, 1, false);

			// Assert:
			assertThat(value, equalTo(0xCDL));
		}

		@Test
		void readsFromOffsetUint16() {
			// Arrange:
			final byte[] bytes = {
					0x11, 0x22, (byte) 0xCD, (byte) 0xAB
			};

			// Act:
			final long value = Converter.bytesToInt(bytes, 2, 2, false);

			// Assert:
			assertThat(value, equalTo(0xABCDL));
		}

		@Test
		void readsFromOffsetUint32() {
			// Arrange:
			final byte[] bytes = {
					0x11, 0x22, 0x12, 0x34, 0x56, 0x78
			};

			// Act:
			final long value = Converter.bytesToInt(bytes, 2, 4, false);

			// Assert:
			assertThat(value, equalTo(0x78563412L));
		}

		@Test
		void readsFromOffsetSigned() {
			// Arrange:
			final byte[] bytes = {
					0x11, 0x22, (byte) 0xFF, (byte) 0xFF
			};

			// Act:
			final long value = Converter.bytesToInt(bytes, 2, 2, true);

			// Assert:
			assertThat(value, equalTo(-1L));
		}

		@Test
		void cannotReadUnsupported1ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToInt(new byte[]{
					0x01
			}, 3, false));
		}

		@Test
		void cannotReadUnsupported8ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToInt(new byte[8], 8, false));
		}

		@Test
		void cannotReadUnsupportedZeroSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToInt(new byte[]{}, 0, false));
		}
	}

	@Nested
	final class BytesToBigInt {
		@Test
		void readsUint64() {
			// Arrange:
			final byte[] bytes = {
					0x01, 0x23, 0x45, 0x67, (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF
			};

			// Act + Assert:
			assertThat(Converter.bytesToBigInt(bytes, 8), equalTo(new BigInteger("EFCDAB8967452301", 16)));
		}

		@Test
		void readsInt64Negative() {
			// Arrange:
			final byte[] bytes = {
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			};

			// Act + Assert:
			assertThat(Converter.bytesToBigInt(bytes, 8, true), equalTo(BigInteger.valueOf(-1)));
		}

		@Test
		void readsUint64MaxValue() {
			// Arrange:
			final byte[] bytes = {
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			};

			// Act:
			final BigInteger value = Converter.bytesToBigInt(bytes, 8, false);

			// Assert:
			assertThat(value, equalTo(new BigInteger("FFFFFFFFFFFFFFFF", 16)));
		}

		@Test
		void readsInt64Positive() {
			// Arrange:
			final byte[] bytes = {
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00
			};

			// Act:
			final BigInteger value = Converter.bytesToBigInt(bytes, 8, true);

			// Assert:
			assertThat(value.signum(), equalTo(1)); // positive
		}

		@Test
		void readsFromOffsetUint64() {
			// Arrange:
			final byte[] bytes = {
					0x11, 0x22, 0x01, 0x23, 0x45, 0x67, (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF
			};

			// Act:
			final BigInteger value = Converter.bytesToBigInt(bytes, 2, 8, false);

			// Assert:
			assertThat(value, equalTo(new BigInteger("EFCDAB8967452301", 16)));
		}

		@Test
		void readsFromOffsetSigned() {
			// Arrange:
			final byte[] bytes = {
					0x11, 0x22, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			};

			// Act:
			final BigInteger value = Converter.bytesToBigInt(bytes, 2, 8, true);

			// Assert:
			assertThat(value, equalTo(BigInteger.valueOf(-1)));
		}

		@Test
		void cannotReadUnsupported4ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToBigInt(new byte[4], 4, false));
		}

		@Test
		void cannotReadUnsupported16ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToBigInt(new byte[16], 16, false));
		}
	}

	@Nested
	final class IntToBytes {
		@Test
		void writesUint8() {
			// Act + Assert:
			assertThat(Converter.intToBytes(0xCDL, 1), equalTo(new byte[]{
					(byte) 0xCD
			}));
		}

		@Test
		void writesUint16() {
			// Act + Assert:
			assertThat(Converter.intToBytes(0xABCDL, 2), equalTo(new byte[]{
					(byte) 0xCD, (byte) 0xAB
			}));
		}

		@Test
		void writesUint32() {
			// Act + Assert:
			assertThat(Converter.intToBytes(0x78563412L, 4), equalTo(new byte[]{
					0x12, 0x34, 0x56, 0x78
			}));
		}

		@Test
		void writesInt8Negative() {
			// Arrange: -1 -> 0xFF
			// Act + Assert:
			assertThat(Converter.intToBytes(-1L, 1, true), equalTo(new byte[]{
					(byte) 0xFF
			}));
		}

		@Test
		void writesInt16Negative() {
			// Arrange: -1 -> 0xFF 0xFF
			// Act + Assert:
			assertThat(Converter.intToBytes(-1L, 2, true), equalTo(new byte[]{
					(byte) 0xFF, (byte) 0xFF
			}));
		}

		@Test
		void writesInt32Negative() {
			// Arrange: -1 -> 0xFF 0xFF 0xFF 0xFF
			// Act + Assert:
			assertThat(Converter.intToBytes(-1L, 4, true), equalTo(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			}));
		}

		@Test
		void writesUint64() {
			// Act + Assert:
			assertThat(Converter.intToBytes(0x0807060504030201L, 8), equalTo(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
			}));
		}

		@Test
		void writesInt64Negative() {
			// Arrange: -1 -> 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
			// Act + Assert:
			assertThat(Converter.intToBytes(-1L, 8, true), equalTo(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			}));
		}

		@Test
		void writesBigIntUint64() {
			// Arrange:
			final byte[] expected = {
					0x01, 0x23, 0x45, 0x67, (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF
			};

			// Act + Assert:
			assertThat(Converter.intToBytes(new BigInteger("EFCDAB8967452301", 16), 8), equalTo(expected));
		}

		@Test
		void writesBigIntInt64Negative() {
			// Arrange:
			final byte[] expected = {
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			};

			// Act + Assert:
			assertThat(Converter.intToBytes(BigInteger.valueOf(-1), 8, true), equalTo(expected));
		}

		@Test
		void writesBigIntUint8() {
			// Act + Assert:
			assertThat(Converter.intToBytes(BigInteger.valueOf(0xCDL), 1), equalTo(new byte[]{
					(byte) 0xCD
			}));
		}

		@Test
		void writesBigIntUint16() {
			// Act + Assert:
			assertThat(Converter.intToBytes(BigInteger.valueOf(0xABCDL), 2), equalTo(new byte[]{
					(byte) 0xCD, (byte) 0xAB
			}));
		}

		@Test
		void writesBigIntUint32() {
			// Act + Assert:
			assertThat(Converter.intToBytes(BigInteger.valueOf(0x78563412L), 4), equalTo(new byte[]{
					0x12, 0x34, 0x56, 0x78
			}));
		}

		@Test
		void cannotWriteUnsupported3ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.intToBytes(100L, 3, false));
		}

		@Test
		void cannotWriteUnsupported5ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.intToBytes(100L, 5, false));
		}

		@Test
		void cannotWriteUnsupported7ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.intToBytes(100L, 7, false));
		}

		@Test
		void cannotWriteUnsupported16ByteSize() {
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> Converter.intToBytes(BigInteger.ONE, 16, false));
		}

		@Test
		void truncatesValuesLargerThanByteSize() {
			// Arrange: 0x123456789ABCDEF0 -> truncate to 4 bytes -> 0x9ABCDEF0
			// Act:
			final byte[] bytes = Converter.intToBytes(0x123456789ABCDEF0L, 4);

			// Assert:
			assertThat(bytes, equalTo(new byte[]{
					(byte) 0xF0, (byte) 0xDE, (byte) 0xBC, (byte) 0x9A
			}));
		}

		@Test
		void zeroValue() {
			// Act:
			final byte[] bytes = Converter.intToBytes(0L, 8);

			// Assert:
			assertThat(bytes, equalTo(new byte[]{
					0, 0, 0, 0, 0, 0, 0, 0
			}));
		}
	}
}
