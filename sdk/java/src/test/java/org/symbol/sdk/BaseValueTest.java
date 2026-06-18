package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.math.BigInteger;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

/**
 * Exercises the {@link BaseValue} base class through a fixture subclass carrying the size/signedness combo, plus the static
 * {@code requireRange} / {@code toHexString} helpers.
 */
final class BaseValueTest {
	/** Fixture: a BaseValue that carries the size/signedness explicitly (everything else is inherited). */
	private static final class Bv extends BaseValue<Bv> {
		Bv(final BigInteger value, final int size, final boolean isSigned) {
			super(value, size, isSigned);
		}
	}

	/** A distinct BaseValue subclass with the same width/signedness as {@link Bv}, used to verify cross-type inequality. */
	private static final class OtherBv extends BaseValue<OtherBv> {
		OtherBv(final BigInteger value, final int size, final boolean isSigned) {
			super(value, size, isSigned);
		}
	}

	private static void canCreateUnsigned(final BigInteger raw, final int size, final BigInteger expected) {
		final Bv v = new Bv(raw, size, false);
		assertThat(v.size(), equalTo(size));
		assertThat(v.value(), equalTo(expected));
	}

	private static void canCreateSigned(final BigInteger raw, final int size, final BigInteger expected) {
		final Bv v = new Bv(raw, size, true);
		assertThat(v.size(), equalTo(size));
		assertThat(v.value(), equalTo(expected));
	}

	// region Creation and Validation

	@Nested
	final class Creation {
		@Test
		void canCreateUnsignedBaseValueAcrossSizes() {
			// Act + Assert:
			for (long raw : new long[]{
					0, 0x24, 0xFF
			})
				canCreateUnsigned(BigInteger.valueOf(raw), 1, BigInteger.valueOf(raw));

			for (long raw : new long[]{
					0, 0x243F, 0xFFFF
			})
				canCreateUnsigned(BigInteger.valueOf(raw), 2, BigInteger.valueOf(raw));

			for (long raw : new long[]{
					0, 0x243F6A88L, 0xFFFFFFFFL
			})
				canCreateUnsigned(BigInteger.valueOf(raw), 4, BigInteger.valueOf(raw));

			final BigInteger[] raw64 = {
					BigInteger.ZERO, new BigInteger("243F6A8885A308D3", 16), new BigInteger("FFFFFFFFFFFFFFFF", 16)
			};

			for (BigInteger raw : raw64)
				canCreateUnsigned(raw, 8, raw);
		}

		@Test
		void cannotCreateUnsignedWithValuesOutsideRange() {
			// Act + Assert:
			assertOutOfRange(() -> new Bv(BigInteger.valueOf(-1), 1, false), "8-bit");
			assertOutOfRange(() -> new Bv(BigInteger.valueOf(0x100), 1, false), "8-bit");

			assertOutOfRange(() -> new Bv(BigInteger.valueOf(-1), 2, false), "16-bit");
			assertOutOfRange(() -> new Bv(BigInteger.valueOf(0x10000), 2, false), "16-bit");

			assertOutOfRange(() -> new Bv(BigInteger.valueOf(-1), 4, false), "32-bit");
			assertOutOfRange(() -> new Bv(BigInteger.valueOf(0x100000000L), 4, false), "32-bit");

			assertOutOfRange(() -> new Bv(BigInteger.valueOf(-1), 8, false), "64-bit");
			assertOutOfRange(() -> new Bv(new BigInteger("10000000000000000", 16), 8, false), "64-bit");
		}

		@Test
		void canCreateSignedBaseValueAcrossSizes() {
			// Act + Assert:
			canCreateSigned(BigInteger.valueOf(-0x80), 1, BigInteger.valueOf(-0x80));
			canCreateSigned(BigInteger.valueOf(0x7F), 1, BigInteger.valueOf(0x7F));

			canCreateSigned(BigInteger.valueOf(-0x8000), 2, BigInteger.valueOf(-0x8000));
			canCreateSigned(BigInteger.valueOf(0x7FFF), 2, BigInteger.valueOf(0x7FFF));

			canCreateSigned(BigInteger.valueOf(-0x80000000L), 4, BigInteger.valueOf(-0x80000000L));
			canCreateSigned(BigInteger.valueOf(0x7FFFFFFFL), 4, BigInteger.valueOf(0x7FFFFFFFL));

			final BigInteger min64 = BigInteger.ONE.shiftLeft(63).negate();
			final BigInteger max64 = BigInteger.ONE.shiftLeft(63).subtract(BigInteger.ONE);
			canCreateSigned(min64, 8, min64);
			canCreateSigned(max64, 8, max64);
		}

		@Test
		void cannotCreateSignedWithValuesOutsideRange() {
			// Act + Assert:
			assertOutOfRange(() -> new Bv(BigInteger.valueOf(-0x81), 1, true), "8-bit");
			assertOutOfRange(() -> new Bv(BigInteger.valueOf(0x80), 1, true), "8-bit");

			assertOutOfRange(() -> new Bv(BigInteger.ONE.shiftLeft(63).negate().subtract(BigInteger.ONE), 8, true), "64-bit");
			assertOutOfRange(() -> new Bv(BigInteger.ONE.shiftLeft(63), 8, true), "64-bit");
		}
	}

	// endregion

	// region Accessors

	@Nested
	final class Accessors {
		@Test
		void valueReturnsUnderlyingValue() {
			// Arrange:
			final Bv v = new Bv(BigInteger.valueOf(42), 4, false);

			// Act + Assert:
			assertThat(v.value(), equalTo(BigInteger.valueOf(42)));
		}

		@Test
		void sizeReturnsByteWidth() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(5), 1, false);
			final Bv v2 = new Bv(BigInteger.valueOf(5), 2, false);
			final Bv v4 = new Bv(BigInteger.valueOf(5), 4, false);
			final Bv v8 = new Bv(BigInteger.valueOf(5), 8, false);

			// Act + Assert:
			assertThat(v1.size(), equalTo(1));
			assertThat(v2.size(), equalTo(2));
			assertThat(v4.size(), equalTo(4));
			assertThat(v8.size(), equalTo(8));
		}

		@Test
		void isSignedReturnsFalseForUnsigned() {
			// Arrange:
			final Bv v = new Bv(BigInteger.valueOf(42), 4, false);

			// Act + Assert:
			assertThat(v.isSigned(), equalTo(false));
		}

		@Test
		void isSignedReturnsTrueForSigned() {
			// Arrange:
			final Bv v = new Bv(BigInteger.valueOf(42), 4, true);

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
			final Bv v1 = new Bv(BigInteger.valueOf(0xFF), 1, false);
			final Bv v4 = new Bv(BigInteger.valueOf(0x12345678L), 4, false);

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
			final Bv v = new Bv(BigInteger.valueOf(0x7F), 1, true);

			// Act:
			final byte[] bytes = v.serialize();

			// Assert:
			assertThat(bytes.length, equalTo(1));
			assertThat(bytes[0], equalTo((byte) 0x7F));
		}

		@Test
		void serializeSignedNegativeValues() {
			// Arrange:
			final Bv v = new Bv(BigInteger.valueOf(-1), 1, true);

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
		void toStringOfUnsignedBaseValuesOutputsFixedWidthHex() {
			// Act + Assert:
			assertThat(new Bv(BigInteger.ZERO, 1, false).toString(), equalTo("0x00"));
			assertThat(new Bv(BigInteger.valueOf(0xFF), 1, false).toString(), equalTo("0xFF"));
			assertThat(new Bv(BigInteger.valueOf(0x12345678L), 4, false).toString(), equalTo("0x12345678"));
			assertThat(new Bv(new BigInteger("1234567890ABCDEF", 16), 8, false).toString(), equalTo("0x1234567890ABCDEF"));
			assertThat(new Bv(new BigInteger("FFFFFFFFFFFFFFFF", 16), 8, false).toString(), equalTo("0xFFFFFFFFFFFFFFFF"));
		}

		@Test
		void toStringOfSignedBaseValuesOutputsFixedWidthHex() {
			// Act + Assert:
			assertThat(new Bv(BigInteger.valueOf(-128), 1, true).toString(), equalTo("0x80"));
			assertThat(new Bv(BigInteger.valueOf(-5), 1, true).toString(), equalTo("0xFB"));
			assertThat(new Bv(BigInteger.valueOf(-1), 1, true).toString(), equalTo("0xFF"));

			assertThat(new Bv(BigInteger.valueOf(-1), 8, true).toString(), equalTo("0xFFFFFFFFFFFFFFFF"));
			assertThat(new Bv(BigInteger.ONE.shiftLeft(63).negate(), 8, true).toString(), equalTo("0x8000000000000000"));
		}

		@Test
		void toStringOfSignedPositiveBaseValueOutputsHexWithoutTwosComplement() {
			// Act + Assert:
			assertThat(new Bv(BigInteger.valueOf(0x7F), 1, true).toString(), equalTo("0x7F"));
			assertThat(new Bv(BigInteger.valueOf(0x1234), 2, true).toString(), equalTo("0x1234"));
		}

		@Test
		void toHexStringStaticMethodWorksForUnsigned() {
			// Act:
			final String hex1 = BaseValue.toHexString(BigInteger.ZERO, 1, false);
			final String hex4 = BaseValue.toHexString(BigInteger.valueOf(0x12345678L), 4, false);

			// Assert:
			assertThat(hex1, equalTo("0x00"));
			assertThat(hex4, equalTo("0x12345678"));
		}

		@Test
		void toHexStringStaticMethodWorksForSigned() {
			// Act:
			final String hex1 = BaseValue.toHexString(BigInteger.valueOf(-1), 1, true);
			final String hex8 = BaseValue.toHexString(BigInteger.valueOf(-1), 8, true);

			// Assert:
			assertThat(hex1, equalTo("0xFF"));
			assertThat(hex8, equalTo("0xFFFFFFFFFFFFFFFF"));
		}
	}

	// endregion

	// region JSON Representation

	@Nested
	final class JsonRepresentation {
		@Test
		void toJsonReturnsNumberBelowEightBytes() {
			// Act + Assert:
			assertThat(new Bv(BigInteger.valueOf(42), 1, false).toJson(), equalTo(42L));
			assertThat(new Bv(BigInteger.valueOf(42), 4, false).toJson(), equalTo(42L));
		}

		@Test
		void toJsonReturnsBase10StringForEightBytes() {
			// Act + Assert:
			assertThat(new Bv(new BigInteger("FFFFFFFFFFFFFFFF", 16), 8, false).toJson(), equalTo("18446744073709551615"));
		}

		@Test
		void toJsonReturnsStringForLargeSignedValues() {
			// Arrange:
			final BigInteger max64 = BigInteger.ONE.shiftLeft(63).subtract(BigInteger.ONE);

			// Act:
			final Object json = new Bv(max64, 8, true).toJson();

			// Assert:
			assertThat(json instanceof String, equalTo(true));
		}
	}

	// endregion

	// region Equality and Hashing

	@Nested
	final class EqualityAndHashing {
		@Test
		void equalsDistinguishesSizeSignednessAndValue() {
			// Arrange:
			final Bv base = new Bv(BigInteger.valueOf(5), 4, false);

			// Act + Assert:
			assertThat(base.equals(base), equalTo(true)); // reflexive
			assertThat(base.equals("not a base value"), equalTo(false)); // non-BaseValue
			assertThat(base.equals(new Bv(BigInteger.valueOf(5), 8, false)), equalTo(false)); // size differs
			assertThat(base.equals(new Bv(BigInteger.valueOf(5), 4, true)), equalTo(false)); // signedness differs
			assertThat(base.equals(new Bv(BigInteger.valueOf(6), 4, false)), equalTo(false)); // value differs
			assertThat(base.equals(null), equalTo(false)); // null
			assertThat(base.equals(new Bv(BigInteger.valueOf(5), 4, false)), equalTo(true)); // all equal
		}

		@Test
		void equalsReturnsFalseForDifferentSubtypeWithSameWidthAndValue() {
			// Arrange: two distinct BaseValue subtypes with identical width, signedness and value
			final Bv base = new Bv(BigInteger.valueOf(5), 4, false);
			final OtherBv other = new OtherBv(BigInteger.valueOf(5), 4, false);

			// Act + Assert: equality is type-discriminating, like the reference SDK's per-type tag
			assertThat(base.equals(other), equalTo(false));
			assertThat(other.equals(base), equalTo(false));
		}

		@Test
		void hashCodeComputedFromSizeSignednessAndValue() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(42), 4, false);
			final Bv v2 = new Bv(BigInteger.valueOf(42), 4, false);
			final Bv v3 = new Bv(BigInteger.valueOf(42), 8, false);

			// Act + Assert:
			assertThat(v1.hashCode(), equalTo(v2.hashCode())); // same parameters
			// v3 should differ (different size)
			boolean hashesEqual = v1.hashCode() == v3.hashCode();
			assertThat(hashesEqual, equalTo(false));
		}

		@Test
		void hashCodeConsistentWithEquals() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(5), 4, false);
			final Bv v2 = new Bv(BigInteger.valueOf(5), 4, false);

			// Act + Assert:
			if (v1.equals(v2))
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
			final Bv v1 = new Bv(BigInteger.valueOf(42), 4, false);
			final Bv v2 = new Bv(BigInteger.valueOf(42), 4, false);

			// Act + Assert:
			assertThat(v1.compareTo(v2), equalTo(0));
		}

		@Test
		void compareToReturnsNegativeWhenLess() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(10), 4, false);
			final Bv v2 = new Bv(BigInteger.valueOf(20), 4, false);

			// Act + Assert:
			assertThat(v1.compareTo(v2) < 0, equalTo(true));
		}

		@Test
		void compareToReturnsPositiveWhenGreater() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(30), 4, false);
			final Bv v2 = new Bv(BigInteger.valueOf(20), 4, false);

			// Act + Assert:
			assertThat(v1.compareTo(v2) > 0, equalTo(true));
		}

		@Test
		void compareToWorksWithSignedNegativeValues() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(-10), 4, true);
			final Bv v2 = new Bv(BigInteger.valueOf(10), 4, true);

			// Act + Assert:
			assertThat(v1.compareTo(v2) < 0, equalTo(true));
		}

		@Test
		void compareToIgnoresSizeAndSignedness() {
			// Arrange:
			final Bv v1 = new Bv(BigInteger.valueOf(42), 4, false);
			final Bv v2 = new Bv(BigInteger.valueOf(42), 8, true);

			// Act + Assert:
			assertThat(v1.compareTo(v2), equalTo(0)); // only values matter
		}
	}

	// endregion

	// region Static Type Converters

	@Nested
	final class StaticTypeConverters {
		@Test
		void toBigIntegerAcceptsNumericTypesAndHexStrings() {
			// Act + Assert:
			assertThat(BaseValue.toBigInteger((short) 7), equalTo(BigInteger.valueOf(7)));
			assertThat(BaseValue.toBigInteger((byte) 8), equalTo(BigInteger.valueOf(8)));
			assertThat(BaseValue.toBigInteger("0x1A"), equalTo(BigInteger.valueOf(26)));
			assertThat(BaseValue.toBigInteger("0X1a"), equalTo(BigInteger.valueOf(26)));
			assertThat(BaseValue.toBigInteger("42"), equalTo(BigInteger.valueOf(42)));
			assertThat(BaseValue.toBigInteger(BigInteger.TEN), equalTo(BigInteger.TEN));
		}

		@Test
		void toBigIntegerAcceptsInteger() {
			// Act + Assert:
			assertThat(BaseValue.toBigInteger(123), equalTo(BigInteger.valueOf(123)));
		}

		@Test
		void toBigIntegerAcceptsLong() {
			// Act + Assert:
			assertThat(BaseValue.toBigInteger(456L), equalTo(BigInteger.valueOf(456L)));
		}

		@Test
		void toBigIntegerRejectsBadStringAndUnsupportedType() {
			// Act + Assert:
			assertThrows(InvalidDescriptorException.class, () -> BaseValue.toBigInteger("not-a-number"));
			assertThrows(InvalidDescriptorException.class, () -> BaseValue.toBigInteger(new Object()));
			assertThrows(InvalidDescriptorException.class, () -> BaseValue.toBigInteger(null));
		}

		@Test
		void toIntConvertsValue() {
			// Act + Assert:
			assertThat(BaseValue.toInt(42), equalTo(42));
			assertThat(BaseValue.toInt("100"), equalTo(100));
			assertThat(BaseValue.toInt("0xFF"), equalTo(255));
		}

		@Test
		void toIntRejectsValueThatDoesNotFit() {
			// Act + Assert:
			assertThrows(InvalidDescriptorException.class, () -> BaseValue.toInt(Long.MAX_VALUE));
			assertThrows(InvalidDescriptorException.class, () -> BaseValue.toInt(new BigInteger("100000000", 16)));
		}

		@Test
		void toLongConvertsValue() {
			// Act + Assert:
			assertThat(BaseValue.toLong(42), equalTo(42L));
			assertThat(BaseValue.toLong("100"), equalTo(100L));
			assertThat(BaseValue.toLong("0x7FFFFFFFFFFFFFFF"), equalTo(Long.MAX_VALUE));
		}

		@Test
		void toLongRejectsValueThatDoesNotFit() {
			// Act + Assert:
			assertThrows(InvalidDescriptorException.class, () -> BaseValue.toLong(new BigInteger("FFFFFFFFFFFFFFFFFF", 16)));
		}
	}

	// endregion

	private static void assertOutOfRange(final Runnable r, final String bitWidth) {
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, r::run);
		assertThat(ex.getMessage(), containsString(bitWidth));
	}
}
