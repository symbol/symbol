package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

/**
 * Exercises the {@link BaseValue} base class through a fixture subclass carrying the size/signedness combo, plus the static
 * {@code requireRange} / {@code toHexString} / converter helpers. The backing value is a {@code long}; unsigned 8-byte values
 * {@code >= 2^63} are the negative two's-complement pattern.
 */
final class BaseValueTest {
	/** Fixture: a BaseValue that carries the size/signedness explicitly (everything else is inherited). */
	private static final class Bv extends BaseValue<Bv> {
		Bv(final long value, final int size, final boolean isSigned) {
			super(value, size, isSigned);
		}
	}

	/** A distinct BaseValue subclass with the same width/signedness as {@link Bv}, used to verify cross-type inequality. */
	private static final class OtherBv extends BaseValue<OtherBv> {
		OtherBv(final long value, final int size, final boolean isSigned) {
			super(value, size, isSigned);
		}
	}

	private static void canCreateUnsigned(final long raw, final int size, final long expected) {
		// Act:
		final Bv v = new Bv(raw, size, false);

		// Assert:
		assertThat(v.size(), equalTo(size));
		assertThat(v.value(), equalTo(expected));
	}

	private static void canCreateSigned(final long raw, final int size, final long expected) {
		// Act:
		final Bv v = new Bv(raw, size, true);

		// Assert:
		assertThat(v.size(), equalTo(size));
		assertThat(v.value(), equalTo(expected));
	}

	private static void assertOutOfRange(final Runnable r, final String bitWidth) {
		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, r::run);

		// Assert:
		assertThat(ex.getMessage(), containsString(bitWidth));
	}

	private static void assertToStringEquals(final long value, final int size, final boolean isSigned, final String expected) {
		// Act:
		final String actual = new Bv(value, size, isSigned).toString();

		// Assert:
		assertThat(actual, equalTo(expected));
	}

	private static void assertEquality(final Object value, final Object other, final boolean expected) {
		// Act:
		final boolean actual = value.equals(other);

		// Assert:
		assertThat(actual, equalTo(expected));
	}

	// region Creation and Validation

	@Nested
	final class Creation {
		@Test
		void canCreateUnsignedByte() {
			for (long raw : new long[]{
					0, 0x24, 0xFF
			})
				canCreateUnsigned(raw, 1, raw);
		}

		@Test
		void canCreateUnsignedShort() {
			for (long raw : new long[]{
					0, 0x243F, 0xFFFF
			})
				canCreateUnsigned(raw, 2, raw);
		}

		@Test
		void canCreateUnsignedInt() {
			for (long raw : new long[]{
					0, 0x243F6A88L, 0xFFFFFFFFL
			})
				canCreateUnsigned(raw, 4, raw);
		}

		@Test
		void canCreateUnsignedLong() {
			// 8-byte u64 values >= 2^63 are stored as the negative bit pattern (0xFFFF...FFFF == -1L)
			for (long raw : new long[]{
					0L, 0x243F6A8885A308D3L, 0xFFFFFFFFFFFFFFFFL
			})
				canCreateUnsigned(raw, 8, raw);
		}

		@Test
		void cannotCreateUnsignedByteOutsideRange() {
			assertOutOfRange(() -> new Bv(-1L, 1, false), "8-bit");
			assertOutOfRange(() -> new Bv(0x100L, 1, false), "8-bit");
		}

		@Test
		void cannotCreateUnsignedShortOutsideRange() {
			assertOutOfRange(() -> new Bv(-1L, 2, false), "16-bit");
			assertOutOfRange(() -> new Bv(0x10000L, 2, false), "16-bit");
		}

		@Test
		void cannotCreateUnsignedIntOutsideRange() {
			assertOutOfRange(() -> new Bv(-1L, 4, false), "32-bit");
			assertOutOfRange(() -> new Bv(0x100000000L, 4, false), "32-bit");
		}

		@Test
		void cannotCreateWithUnsupportedSize() {
			// only the fixed catbuffer widths {1,2,4,8} are valid: 0 / >8 would wrap the bound math (8*size exceeds a long,
			// and Java masks shift distances), and 3/5/6/7 would construct but then throw at serialize() (validateByteSize) —
			// reject all of them up front with a consistent message
			for (final int size : new int[]{
					0, 3, 5, 6, 7, 9
			})
				assertOutOfRange(() -> new Bv(0L, size, false), "must be 1, 2, 4 or 8 bytes");
		}

		@Test
		void canCreateSignedByte() {
			canCreateSigned(-0x80L, 1, -0x80L);
			canCreateSigned(0x7FL, 1, 0x7FL);
		}

		@Test
		void canCreateSignedShort() {
			canCreateSigned(-0x8000L, 2, -0x8000L);
			canCreateSigned(0x7FFFL, 2, 0x7FFFL);
		}

		@Test
		void canCreateSignedInt() {
			canCreateSigned(-0x80000000L, 4, -0x80000000L);
			canCreateSigned(0x7FFFFFFFL, 4, 0x7FFFFFFFL);
		}

		@Test
		void canCreateSignedLong() {
			canCreateSigned(Long.MIN_VALUE, 8, Long.MIN_VALUE);
			canCreateSigned(Long.MAX_VALUE, 8, Long.MAX_VALUE);
		}

		// note: there is no signed long (size 8) out-of-range case — it admits any 64-bit pattern

		@Test
		void cannotCreateSignedByteOutsideRange() {
			assertOutOfRange(() -> new Bv(-0x81L, 1, true), "8-bit");
			assertOutOfRange(() -> new Bv(0x80L, 1, true), "8-bit");
		}

		@Test
		void cannotCreateSignedIntOutsideRange() {
			assertOutOfRange(() -> new Bv(-0x80000001L, 4, true), "32-bit");
			assertOutOfRange(() -> new Bv(0x80000000L, 4, true), "32-bit");
		}
	}

	// endregion

	// region Accessors

	@Nested
	final class Accessors {
		@Test
		void valueReturnsUnderlyingValue() {
			// Arrange:
			final Bv v = new Bv(42L, 4, false);

			// Act + Assert:
			assertThat(v.value(), equalTo(42L));
		}

		@ParameterizedTest
		@ValueSource(ints = {
				1, 2, 4, 8
		})
		void sizeReturnsByteWidth(final int size) {
			assertThat(new Bv(5L, size, false).size(), equalTo(size));
		}

		@Test
		void isSignedReturnsFalseForUnsigned() {
			// Arrange:
			final Bv v = new Bv(42L, 4, false);

			// Act + Assert:
			assertThat(v.isSigned(), equalTo(false));
		}

		@Test
		void isSignedReturnsTrueForSigned() {
			// Arrange:
			final Bv v = new Bv(42L, 4, true);

			// Act + Assert:
			assertThat(v.isSigned(), equalTo(true));
		}
	}

	// endregion

	// region Serialization

	@Nested
	final class Serialization {
		@Test
		void serializeUnsignedValues() {
			// Arrange:
			final Bv v1 = new Bv(0xFFL, 1, false);
			final Bv v4 = new Bv(0x12345678L, 4, false);

			// Act:
			final byte[] bytes1 = v1.serialize();
			final byte[] bytes4 = v4.serialize();

			// Assert:
			assertThat(bytes1.length, equalTo(1));
			assertThat(bytes4.length, equalTo(4));
			assertThat(bytes1, equalTo(new byte[]{
					(byte) 0xFF
			}));
			assertThat(bytes4, equalTo(new byte[]{
					0x78, 0x56, 0x34, 0x12
			}));
		}

		@Test
		void serializeSignedPositiveValues() {
			// Arrange:
			final Bv v = new Bv(0x7FL, 1, true);

			// Act:
			final byte[] bytes = v.serialize();

			// Assert:
			assertThat(bytes.length, equalTo(1));
			assertThat(bytes[0], equalTo((byte) 0x7F));
		}

		@Test
		void serializeSignedNegativeValues() {
			// Arrange:
			final Bv v = new Bv(-1L, 1, true);

			// Act:
			final byte[] bytes = v.serialize();

			// Assert:
			assertThat(bytes.length, equalTo(1));
			assertThat(bytes[0], equalTo((byte) 0xFF));
		}
	}

	// endregion

	// region String Representation

	@Nested
	final class StringRepresentation {
		@Test
		void toStringOfUnsignedByte() {
			assertToStringEquals(0L, 1, false, "0x00");
			assertToStringEquals(0xFFL, 1, false, "0xFF");
		}

		@Test
		void toStringOfUnsignedInt() {
			assertToStringEquals(0x12345678L, 4, false, "0x12345678");
		}

		@Test
		void toStringOfUnsignedLong() {
			assertToStringEquals(0x1234567890ABCDEFL, 8, false, "0x1234567890ABCDEF");
			assertToStringEquals(0xFFFFFFFFFFFFFFFFL, 8, false, "0xFFFFFFFFFFFFFFFF");
		}

		@Test
		void toStringOfSignedByte() {
			assertToStringEquals(-128L, 1, true, "0x80");
			assertToStringEquals(-5L, 1, true, "0xFB");
			assertToStringEquals(-1L, 1, true, "0xFF");
		}

		@Test
		void toStringOfSignedLong() {
			assertToStringEquals(-1L, 8, true, "0xFFFFFFFFFFFFFFFF");
			assertToStringEquals(Long.MIN_VALUE, 8, true, "0x8000000000000000");
		}

		@Test
		void toStringOfSignedPositiveBaseValueOutputsHexWithoutTwosComplement() {
			assertToStringEquals(0x7FL, 1, true, "0x7F");
			assertToStringEquals(0x1234L, 2, true, "0x1234");
		}

		@Test
		void toHexStringStaticMethodWorksForUnsigned() {
			// Act:
			final String hex1 = BaseValue.toHexString(0L, 1, false);
			final String hex4 = BaseValue.toHexString(0x12345678L, 4, false);

			// Assert:
			assertThat(hex1, equalTo("0x00"));
			assertThat(hex4, equalTo("0x12345678"));
		}

		@Test
		void toHexStringStaticMethodWorksForSigned() {
			// Act:
			final String hex1 = BaseValue.toHexString(-1L, 1, true);
			final String hex8 = BaseValue.toHexString(-1L, 8, true);

			// Assert:
			assertThat(hex1, equalTo("0xFF"));
			assertThat(hex8, equalTo("0xFFFFFFFFFFFFFFFF"));
		}
	}

	// endregion

	// region JSON Representation

	private static void assertJson(final long value, final int size, final boolean isSigned, final Object expected) {
		assertThat(new Bv(value, size, isSigned).toJson(), equalTo(expected));
	}

	@Nested
	final class JsonRepresentation {
		@Test
		void toJsonOfUnsignedOutputsNumberOrString() {
			// sizes below 8 render as the numeric value; 8-byte values render as an unsigned base-10 string
			assertJson(0L, 1, false, 0L);
			assertJson(0x24L, 1, false, 0x24L);
			assertJson(0xFFL, 1, false, 0xFFL);

			assertJson(0L, 2, false, 0L);
			assertJson(0x1234L, 2, false, 0x1234L);
			assertJson(0xFFFFL, 2, false, 0xFFFFL);

			assertJson(0L, 4, false, 0L);
			assertJson(0x12345678L, 4, false, 0x12345678L);
			assertJson(0xFFFFFFFFL, 4, false, 0xFFFFFFFFL);

			assertJson(0L, 8, false, "0");
			assertJson(0x12345678L, 8, false, "305419896");
			assertJson(0x1234567890ABCDEFL, 8, false, "1311768467294899695");
			assertJson(0xFFFFFFFFFFFFFFFFL, 8, false, "18446744073709551615");
		}

		@Test
		void toJsonOfSignedOutputsNumberOrString() {
			// sizes below 8 render as the (possibly negative) numeric value; 8-byte values render as a signed base-10 string
			assertJson(0L, 1, true, 0L);
			assertJson(127L, 1, true, 127L);
			assertJson(-128L, 1, true, -128L);
			assertJson(-1L, 1, true, -1L);

			assertJson(0x1234L, 2, true, 0x1234L);
			assertJson(0x7FFFL, 2, true, 0x7FFFL);
			assertJson(-0x8000L, 2, true, -0x8000L);
			assertJson(-1L, 2, true, -1L);

			assertJson(0x12345678L, 4, true, 0x12345678L);
			assertJson(0x7FFFFFFFL, 4, true, 0x7FFFFFFFL);
			assertJson(-0x80000000L, 4, true, -0x80000000L);
			assertJson(-1L, 4, true, -1L);

			assertJson(0L, 8, true, "0");
			assertJson(Long.MAX_VALUE, 8, true, "9223372036854775807");
			assertJson(Long.MIN_VALUE, 8, true, "-9223372036854775808");
			assertJson(-5L, 8, true, "-5");
			assertJson(-1L, 8, true, "-1");
		}
	}

	// endregion

	// region Equality and Hashing

	@Nested
	final class EqualityAndHashing {
		@Test
		void equalsReturnsTrueForReflexive() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, base, true);
		}

		@Test
		void equalsReturnsTrueForSameSizeSignednessAndValue() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, new Bv(5L, 4, false), true);
		}

		@Test
		void equalsReturnsFalseForDifferentSize() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, new Bv(5L, 8, false), false);
		}

		@Test
		void equalsReturnsFalseForDifferentSignedness() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, new Bv(5L, 4, true), false);
		}

		@Test
		void equalsReturnsFalseForDifferentValue() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, new Bv(6L, 4, false), false);
		}

		@Test
		void equalsReturnsFalseForNull() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, null, false);
		}

		@Test
		void equalsReturnsFalseForNonBaseValue() {
			// Arrange:
			final Bv base = new Bv(5L, 4, false);

			// Act + Assert:
			assertEquality(base, "not a base value", false);
		}

		@Test
		void equalsReturnsFalseForDifferentSubtypeWithSameWidthAndValue() {
			// Arrange: two distinct BaseValue subtypes with identical width, signedness and value
			final Bv base = new Bv(5L, 4, false);
			final OtherBv other = new OtherBv(5L, 4, false);

			// Act + Assert: equality is type-discriminating, like the reference SDK's per-type tag
			assertEquality(base, other, false);
			assertEquality(other, base, false);
		}

		@Test
		void hashCodeComputedFromSizeSignednessAndValue() {
			// Arrange:
			final Bv v1 = new Bv(42L, 4, false);
			final Bv v2 = new Bv(42L, 4, false);
			final Bv v3 = new Bv(42L, 8, false);

			// Act + Assert: equal values must hash equally (the one property the hashCode contract guarantees)
			assertThat(v1.hashCode(), equalTo(v2.hashCode()));

			// a value differing only in size is a distinct object — assert that at the equals level; pinning distinct hash
			// codes would test an accidental non-collision that the contract does not require (unequal objects may collide)
			assertEquality(v1, v3, false);
		}

		@Test
		void hashCodeConsistentWithEquals() {
			// Arrange:
			final Bv v1 = new Bv(5L, 4, false);
			final Bv v2 = new Bv(5L, 4, false);

			// Act + Assert: equal values must hash equally (asserted unconditionally, not gated on equals)
			assertThat(v1.equals(v2), equalTo(true));
			assertThat(v1.hashCode(), equalTo(v2.hashCode()));
		}
	}

	// endregion

	// region Comparisons

	@Nested
	final class Comparisons {
		@Test
		void compareToReturnsZeroForEqual() {
			// Arrange:
			final Bv v1 = new Bv(42L, 4, false);
			final Bv v2 = new Bv(42L, 4, false);

			// Act + Assert:
			assertThat(v1.compareTo(v2), equalTo(0));
		}

		@Test
		void compareToReturnsNegativeWhenLess() {
			// Arrange:
			final Bv v1 = new Bv(10L, 4, false);
			final Bv v2 = new Bv(20L, 4, false);

			// Act + Assert:
			assertThat(v1.compareTo(v2) < 0, equalTo(true));
		}

		@Test
		void compareToReturnsPositiveWhenGreater() {
			// Arrange:
			final Bv v1 = new Bv(30L, 4, false);
			final Bv v2 = new Bv(20L, 4, false);

			// Act + Assert:
			assertThat(v1.compareTo(v2) > 0, equalTo(true));
		}

		@Test
		void compareToWorksWithSignedNegativeValues() {
			// Arrange:
			final Bv v1 = new Bv(-10L, 4, true);
			final Bv v2 = new Bv(10L, 4, true);

			// Act + Assert:
			assertThat(v1.compareTo(v2) < 0, equalTo(true));
		}

		@Test
		void compareToUnsignedTreatsHighBitValuesAsLarge() {
			// Arrange: an unsigned 8-byte value with the high bit set is the LARGER value, even though its
			// signed-long representation is negative — compareTo must use unsigned ordering.
			final Bv small = new Bv(1L, 8, false);
			final Bv large = new Bv(0xFFFFFFFFFFFFFFFFL, 8, false); // u64 max == -1L

			// Act + Assert:
			assertThat(small.compareTo(large) < 0, equalTo(true));
			assertThat(large.compareTo(small) > 0, equalTo(true));
		}
	}

	// endregion
}
