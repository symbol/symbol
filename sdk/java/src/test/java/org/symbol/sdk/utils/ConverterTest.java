package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

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

			for (String input : inputs) {
				// Act:
				final boolean actual = Converter.isHexString(input);

				// Assert:
				assertThat("input " + input, actual, is(true));
			}
		}

		@Test
		void returnsFalseForInvalidHexStrings() {
			// Arrange:
			final String[] inputs = {
					"abcdef012345G789ABCDEF", // invalid ('G') char
					"abcdef0123456789ABCDE" // invalid (odd) length
			};

			for (String input : inputs) {
				// Act:
				final boolean actual = Converter.isHexString(input);

				// Assert:
				assertThat("input " + input, actual, is(false));
			}
		}

		@Test
		void acceptsEmptyString() {
			// Act:
			final boolean actual = Converter.isHexString("");

			// Assert:
			assertThat(actual, is(true));
		}

		@Test
		void acceptsSingleOctet() {
			assertThat(Converter.isHexString("FF"), is(true));
			assertThat(Converter.isHexString("00"), is(true));
		}

		@Test
		void rejectsSingleCharacter() {
			// Act:
			final boolean actual = Converter.isHexString("F");

			// Assert:
			assertThat(actual, is(false));
		}

		@Test
		void acceptsMixedCase() {
			// Act:
			final boolean actual = Converter.isHexString("AbCdEf0123");

			// Assert:
			assertThat(actual, is(true));
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
			assertThrows(IllegalArgumentException.class, () -> Converter.hexToUint8("026ee415fc1"));
		}

		@Test
		void cannotParseHexStringWithInvalidChar() {
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
			// Act:
			final String actual = Converter.uint8ToHex(new byte[0]);

			// Assert:
			assertThat(actual, equalTo(""));
		}

		@Test
		void canEncodeSingleByte() {
			// Act:
			final String actual = Converter.uint8ToHex(new byte[]{
					(byte) 0xFF
			});

			// Assert:
			assertThat(actual, equalTo("FF"));
		}

		@Test
		void canEncodeZeroBytes() {
			// Act:
			final String actual = Converter.uint8ToHex(new byte[]{
					0x00, 0x00, 0x00
			});

			// Assert:
			assertThat(actual, equalTo("000000"));
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
		private static void assertReads(final byte[] bytes, final int size, final boolean signed, final long expected) {
			assertThat(Converter.bytesToInt(bytes, size, signed), equalTo(expected));
		}

		@Test
		void readsUint8() {
			assertReads(new byte[]{
					(byte) 0xCD
			}, 1, false, 0xCDL);
		}

		@Test
		void readsUint16() {
			assertReads(new byte[]{
					(byte) 0xCD, (byte) 0xAB
			}, 2, false, 0xABCDL);
		}

		@Test
		void readsUint32() {
			assertReads(new byte[]{
					0x12, 0x34, 0x56, 0x78
			}, 4, false, 0x78563412L);
		}

		@Test
		void readsInt8() {
			assertReads(new byte[]{
					(byte) 0xCD
			}, 1, true, -51L);
		}

		@Test
		void readsInt16() {
			assertReads(new byte[]{
					(byte) 0xFF, (byte) 0xFF
			}, 2, true, -1L);
		}

		@Test
		void readsInt16Unsigned() {
			// 0x80 high bit: 0x8000 unsigned reads back as -32768 when signed
			assertReads(new byte[]{
					0x00, (byte) 0x80
			}, 2, true, -32768L);
		}

		@Test
		void readsInt32() {
			assertReads(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			}, 4, true, -1L);
		}

		@Test
		void readsInt32Unsigned() {
			// 0x80 high bit: 0x80000000 unsigned reads back as -2147483648 when signed
			assertReads(new byte[]{
					0x00, 0x00, 0x00, (byte) 0x80
			}, 4, true, -2147483648L);
		}

		@Test
		void readsUint64() {
			// size 8 returns the raw little-endian 64-bit pattern (u64 max reads back as -1L)
			assertReads(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			}, 8, false, -1L);
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
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToInt(new byte[]{
					0x01
			}, 3, false));
		}

		@Test
		void cannotReadUnsupportedZeroSize() {
			assertThrows(IllegalArgumentException.class, () -> Converter.bytesToInt(new byte[]{}, 0, false));
		}
	}

	@Nested
	final class IntToBytes {
		private static void assertWrites(final long value, final int size, final boolean signed, final byte[] expected) {
			assertThat(Converter.intToBytes(value, size, signed), equalTo(expected));
		}

		@Test
		void writesUint8() {
			assertWrites(0xCDL, 1, false, new byte[]{
					(byte) 0xCD
			});
		}

		@Test
		void writesUint16() {
			assertWrites(0xABCDL, 2, false, new byte[]{
					(byte) 0xCD, (byte) 0xAB
			});
		}

		@Test
		void writesUint32() {
			assertWrites(0x78563412L, 4, false, new byte[]{
					0x12, 0x34, 0x56, 0x78
			});
		}

		@Test
		void writesInt8Negative() {
			assertWrites(-1L, 1, true, new byte[]{
					(byte) 0xFF
			});
		}

		@Test
		void writesInt16Negative() {
			assertWrites(-1L, 2, true, new byte[]{
					(byte) 0xFF, (byte) 0xFF
			});
		}

		@Test
		void writesInt32Negative() {
			assertWrites(-1L, 4, true, new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			});
		}

		@Test
		void writesUint64() {
			assertWrites(0x0807060504030201L, 8, false, new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
			});
		}

		@Test
		void writesInt64Negative() {
			assertWrites(-1L, 8, true, new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF
			});
		}

		@ParameterizedTest
		@ValueSource(ints = {
				3, 5, 7
		})
		void cannotWriteUnsupportedByteSize(final int size) {
			assertThrows(IllegalArgumentException.class, () -> Converter.intToBytes(100L, size, false));
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

	@Nested
	final class DescriptorCoercion {
		@Test
		void toLongReturnsPrimitiveValueUnchanged() {
			// Act + Assert: the long overload is an identity pass-through documenting the 64-bit-pattern contract.
			assertThat(Converter.toLong(42L), equalTo(42L));
			assertThat(Converter.toLong(0L), equalTo(0L));
			assertThat(Converter.toLong(-1L), equalTo(-1L));
		}

		@Test
		void toLongParsesDecimalAndHexStringsAcrossFullU64() {
			assertThat(Converter.toLong("255"), equalTo(255L));
			assertThat(Converter.toLong("0xFF"), equalTo(255L));
			assertThat(Converter.toLong("0X1a"), equalTo(26L));
			assertThat(Converter.toLong("0x7FFFFFFFFFFFFFFF"), equalTo(Long.MAX_VALUE));
			// the upper unsigned half (>= 2^63) parses to the negative two's-complement pattern; u64 max -> -1L
			assertThat(Converter.toLong("18446744073709551615"), equalTo(-1L));
			assertThat(Converter.toLong("0xFFFFFFFFFFFFFFFF"), equalTo(-1L));
		}

		@Test
		void toLongStringAcceptsNegativeDecimalsLikeNumber() {
			// a leading '-' is parsed signed, so toLong(String) accepts negatives like toLong(Number)
			assertThat(Converter.toLong("-5"), equalTo(Converter.toLong(Long.valueOf(-5))));
			assertThat(Converter.toLong(String.valueOf(Long.MIN_VALUE)), equalTo(Long.MIN_VALUE));
			// a negative and its unsigned-u64 twin (2^64 - 5) are the same 64-bit pattern
			assertThat(Converter.toLong("-5"), equalTo(Converter.toLong("18446744073709551611")));
		}

		@Test
		void toLongRejectsUnparseableOrOversizedStrings() {
			// Act + Assert: non-numeric text and values wider than 64 bits are rejected as invalid descriptors.
			assertThrows(IllegalArgumentException.class, () -> Converter.toLong("not-a-number"));
			assertThrows(IllegalArgumentException.class, () -> Converter.toLong("99999999999999999999999"));
			assertThrows(IllegalArgumentException.class, () -> Converter.toLong("-99999999999999999999999"));
		}

		@Test
		void toLongConvertsFixedWidthIntegralWrappers() {
			// Act + Assert: Integer/Long/Short/Byte always fit 64 bits and are taken directly.
			assertThat(Converter.toLong(Integer.valueOf(42)), equalTo(42L));
			assertThat(Converter.toLong(Short.valueOf((short) 7)), equalTo(7L));
			assertThat(Converter.toLong(Byte.valueOf((byte) 8)), equalTo(8L));
			assertThat(Converter.toLong(Long.valueOf(-1L)), equalTo(-1L));
		}

		@Test
		void toLongRejectsUnsupportedNumberType() {
			// Act + Assert: only the fixed-width integral wrappers are accepted; any other Number (e.g. a non-integral Double) is rejected.
			assertThrows(IllegalArgumentException.class, () -> Converter.toLong(Double.valueOf(1.5)));
		}

		@Test
		void toLongRejectsNull() {
			// a null Number is rejected with the same IllegalArgumentException as any other non-integral type, not an NPE
			assertThrows(IllegalArgumentException.class, () -> Converter.toLong((Number) null));
		}

		@Test
		void toIntConvertsValuesWithinIntRange() {
			assertThat(Converter.toInt(Integer.valueOf(42)), equalTo(42));
			assertThat(Converter.toInt(Short.valueOf((short) 255)), equalTo(255));
		}

		@Test
		void toIntNumberAcceptsUnsignedU32AndRejectsBeyond() {
			// the signed int range and the full unsigned u32 range are accepted (the upper half returns the two's-complement int)
			assertThat(Converter.toInt(Long.valueOf(0xFFFFFFFFL)), equalTo(-1));
			assertThat(Converter.toInt(Long.valueOf(0x80000000L)), equalTo(Integer.MIN_VALUE));
			// beyond 2^32-1, or below the signed int minimum, is rejected rather than truncated
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt(Long.valueOf(0x1_0000_0000L))); // 2^32
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt(Long.valueOf(Long.MAX_VALUE)));
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt(Long.valueOf(-3_000_000_000L)));
		}

		@Test
		void toIntParsesAndRangeChecksStrings() {
			assertThat(Converter.toInt("255"), equalTo(255));
			assertThat(Converter.toInt("0xFF"), equalTo(255));
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt("4294967296")); // beyond the unsigned 32-bit range
		}

		@Test
		void toIntRejectsMagnitudesAboveUnsignedRange() {
			// values beyond the unsigned 32-bit range are rejected, not truncated to a valid-looking int
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt("18446744073709551615")); // 2^64 - 1
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt("0xFFFFFFFFFFFFFFFF"));
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt("0xFFFFFFFF80000000"));
		}

		@Test
		void toIntStringAcceptsFullUnsignedAndSignedRange() {
			// mirrors toLong: a leading '-' parses signed; hex and non-negative decimal read the full unsigned u32 range, the
			// upper half returning the two's-complement int
			assertThat(Converter.toInt("-5"), equalTo(-5));
			assertThat(Converter.toInt(String.valueOf(Integer.MIN_VALUE)), equalTo(Integer.MIN_VALUE));
			assertThat(Converter.toInt(String.valueOf(Integer.MAX_VALUE)), equalTo(Integer.MAX_VALUE));
			assertThat(Converter.toInt("2147483648"), equalTo(Integer.MIN_VALUE)); // 2^31 as unsigned decimal
			assertThat(Converter.toInt("4294967295"), equalTo(-1)); // 2^32 - 1 as unsigned decimal
			// beyond 2^32-1 is rejected
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt("4294967296"));
		}

		@Test
		void toIntHexAcceptsFullUnsignedRange() {
			// hex reads the full unsigned u32 range; a set bit 31 returns the negative two's-complement int
			assertThat(Converter.toInt("0x7FFFFFFF"), equalTo(Integer.MAX_VALUE));
			assertThat(Converter.toInt("0x80000000"), equalTo(Integer.MIN_VALUE));
			assertThat(Converter.toInt("0xFFFFFFFF"), equalTo(-1));
			// beyond the unsigned 32-bit range is rejected
			assertThrows(IllegalArgumentException.class, () -> Converter.toInt("0x100000000"));
		}
	}
}
