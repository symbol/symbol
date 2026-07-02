package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

import org.symbol.sdk.utils.Converter;

/**
 * Exercises the {@link ByteArray} interface contract through a tiny record fixture.
 */
final class ByteArrayTest {
	private static final int FIXED_SIZE = 24;
	private static final byte[] TEST_BYTES = {
			(byte) 0xC5, (byte) 0xFB, (byte) 0x65, (byte) 0xCB, (byte) 0x90, (byte) 0x26, (byte) 0x23, (byte) 0xD9, (byte) 0x3D,
			(byte) 0xF2, (byte) 0xE6, (byte) 0x82, (byte) 0xFF, (byte) 0xB1, (byte) 0x3F, (byte) 0x99, (byte) 0xD5, (byte) 0x0F,
			(byte) 0xAC, (byte) 0x24, (byte) 0xD5, (byte) 0xFF, (byte) 0x2A, (byte) 0x42
	};
	private static final String TEST_HEX = "C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42";

	/** Minimal fixture: a 24-byte ByteArray subclass (size validation + hex toString inherited from the base). */
	private static final class Fixed extends ByteArray {
		Fixed(final byte[] bytes) {
			super(bytes, FIXED_SIZE);
		}

		Fixed(final String hex) {
			this(Converter.hexToUint8(hex));
		}
	}

	/** A distinct ByteArray subclass with the same size as {@link Fixed}, used to verify cross-type inequality. */
	private static final class OtherFixed extends ByteArray {
		OtherFixed(final byte[] bytes) {
			super(bytes, FIXED_SIZE);
		}
	}

	// region Creation and Validation

	@Nested
	final class CreationAndValidation {
		@Test
		void canCreateByteArrayWithCorrectNumberOfBytes() {
			// Act:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Assert:
			assertThat(ba.bytes(), equalTo(TEST_BYTES));
		}

		@ParameterizedTest
		@ValueSource(ints = {
				0, FIXED_SIZE - 1, FIXED_SIZE + 1
		})
		void cannotCreateByteArrayWithIncorrectNumberOfBytes(final int size) {
			final byte[] payload = new byte[size];
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> new Fixed(payload));
			assertThat(ex.getMessage(), containsString("bytes was size"));
		}

		@Test
		void canCreateByteArrayWithCorrectNumberOfHexCharacters() {
			// Act:
			final Fixed ba = new Fixed(TEST_HEX);

			// Assert:
			assertThat(ba.bytes(), equalTo(TEST_BYTES));
		}

		@ParameterizedTest
		@ValueSource(ints = {
				0, FIXED_SIZE - 1, FIXED_SIZE + 1
		})
		void cannotCreateByteArrayWithIncorrectNumberOfHexCharacters(final int size) {
			final String hex = "AB".repeat(size);
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> new Fixed(hex));
			assertThat(ex.getMessage(), containsString("bytes was size"));
		}
	}

	// endregion

	// region Accessors

	@Nested
	final class Accessors {
		@Test
		void sizeReturnsLengthOfBytes() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act + Assert:
			assertThat(ba.size(), equalTo(FIXED_SIZE));
			assertThat(ba.size(), equalTo(TEST_BYTES.length));
		}

		@Test
		void bytesReturnsInternalBuffer() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act + Assert: bytes() exposes the live internal buffer (no per-call copy), so repeat reads are the same instance
			assertThat(ba.bytes(), equalTo(TEST_BYTES));
			assertThat(ba.bytes(), sameInstance(ba.bytes()));
		}
	}

	// endregion

	// region Defensive Copying

	@Nested
	final class DefensiveCopying {
		@Test
		void constructorDefensivelyCopiesInput() {
			// Arrange:
			final byte[] input = TEST_BYTES.clone();

			// Act:
			final Fixed ba = new Fixed(input);

			// Mutate the input array
			input[0] = (byte) 0x99;

			// Assert: ByteArray should not be affected
			assertThat(ba.bytes()[0], equalTo(TEST_BYTES[0]));
		}

		@Test
		void serializeReturnsDefensiveCopy() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act:
			final byte[] serialized = ba.serialize();
			serialized[0] = (byte) 0x99;

			// Assert: Serialize again - should be unchanged
			final byte[] serialized2 = ba.serialize();
			assertThat(serialized2[0], equalTo(TEST_BYTES[0]));
		}
	}

	// endregion

	// region Serialization

	@Nested
	final class Serialization {
		@Test
		void serializeReturnsCorrectBytes() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act:
			final byte[] serialized = ba.serialize();

			// Assert:
			assertThat(serialized, equalTo(TEST_BYTES));
		}
	}

	// endregion

	// region String Representation

	@Nested
	final class StringRepresentation {
		@Test
		void supportsToString() {
			// Act:
			final String actual = new Fixed(TEST_BYTES).toString();

			// Assert:
			assertThat(actual, equalTo(TEST_HEX));
		}

		@Test
		void supportsToJson() {
			// Act:
			final String actual = new Fixed(TEST_BYTES).toJson();

			// Assert:
			assertThat(actual, equalTo(TEST_HEX));
		}

		@Test
		void toStringAndToJsonAreIdentical() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act + Assert:
			assertThat(ba.toString(), equalTo(ba.toJson()));
		}

		@Test
		void toStringOutputsUppercaseHex() {
			// Arrange:
			final Fixed ba = new Fixed(Converter.hexToUint8("ABcDeF" + "00".repeat(21)));

			// Act:
			final String hex = ba.toString();

			// Assert:
			assertThat(hex.substring(0, 6), equalTo("ABCDEF"));
		}

		@Test
		void toStringReturnsFixedLengthHex() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act:
			final String hex = ba.toString();

			// Assert:
			assertThat(hex.length(), equalTo(FIXED_SIZE * 2));
		}
	}

	// endregion

	// region Equality

	@Nested
	final class Equality {
		@Test
		void equalsReturnsTrue() {
			// Arrange:
			final Fixed ba1 = new Fixed(TEST_BYTES);
			final Fixed ba2 = new Fixed(TEST_HEX);

			// Act + Assert:
			assertThat(ba1.equals(ba2), equalTo(true));
		}

		@Test
		void equalsReturnsFalseForDifferentBytes() {
			// Arrange:
			final byte[] differentBytes = TEST_BYTES.clone();
			differentBytes[0] = (byte) 0x99;
			final Fixed ba1 = new Fixed(TEST_BYTES);
			final Fixed ba2 = new Fixed(differentBytes);

			// Act + Assert:
			assertThat(ba1.equals(ba2), equalTo(false));
		}

		@Test
		void equalsReturnsTrueForReflexivity() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act + Assert:
			assertThat(ba.equals(ba), equalTo(true));
		}

		@Test
		void equalsReturnsFalseForDifferentType() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act + Assert:
			assertThat(ba.equals("not a byte array"), equalTo(false));
			assertThat(ba.equals(TEST_BYTES), equalTo(false));
			assertThat(ba.equals(null), equalTo(false));
		}

		@Test
		void equalsReturnsFalseForDifferentSubtypeWithSameBytes() {
			// Arrange: two distinct ByteArray subtypes wrapping identical bytes
			final Fixed ba = new Fixed(TEST_BYTES);
			final OtherFixed other = new OtherFixed(TEST_BYTES);

			// Act + Assert: equality is type-discriminating, like the reference SDK's per-type tag
			assertThat(ba.equals(other), equalTo(false));
			assertThat(other.equals(ba), equalTo(false));
		}

		@Test
		void equalsIsSymmetricAndTransitive() {
			// Arrange:
			final Fixed ba1 = new Fixed(TEST_BYTES);
			final Fixed ba2 = new Fixed(TEST_HEX);
			final Fixed ba3 = new Fixed(Converter.hexToUint8(TEST_HEX));

			// Act + Assert:
			assertThat(ba1.equals(ba2), equalTo(true));
			assertThat(ba2.equals(ba1), equalTo(true));
			assertThat(ba1.equals(ba3), equalTo(true));
		}
	}

	// endregion

	// region Hashing

	@Nested
	final class Hashing {
		@Test
		void hashCodeIsConsistentWithEquals() {
			// Arrange:
			final Fixed ba1 = new Fixed(TEST_BYTES);
			final Fixed ba2 = new Fixed(TEST_HEX);

			// Act + Assert:
			assertThat(ba1.hashCode(), equalTo(ba2.hashCode()));
		}

		@Test
		void hashCodeIsConsistent() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act:
			final int hash1 = ba.hashCode();
			final int hash2 = ba.hashCode();

			// Assert:
			assertThat(hash1, equalTo(hash2));
		}

		@Test
		void hashCodeDifferentiatesDifferentValues() {
			// Arrange:
			final byte[] differentBytes = TEST_BYTES.clone();
			differentBytes[0] = (byte) 0x99;
			final Fixed ba1 = new Fixed(TEST_BYTES);
			final Fixed ba2 = new Fixed(differentBytes);

			// Act:
			final int hash1 = ba1.hashCode();
			final int hash2 = ba2.hashCode();

			// Assert: Very likely to be different (not guaranteed but highly probable)
			assertThat(hash1, not(equalTo(hash2)));
		}
	}

	// endregion

	// region Static toBytes Converter

	@Nested
	final class StaticToBytesConverter {
		@Test
		void canConvertByteArrayInput() {
			// Act:
			final byte[] result = ByteArray.toBytes(TEST_BYTES);

			// Assert:
			assertThat(result, equalTo(TEST_BYTES));
		}

		@Test
		void canConvertByteArrayInstance() {
			// Arrange:
			final Fixed ba = new Fixed(TEST_BYTES);

			// Act:
			final byte[] result = ByteArray.toBytes(ba);

			// Assert:
			assertThat(result, equalTo(TEST_BYTES));
		}

		@Test
		void canConvertHexString() {
			// Act: a String is read as hex (a byte-array POD's canonical form)
			final byte[] result = ByteArray.toBytes(TEST_HEX);

			// Assert:
			assertThat(result, equalTo(TEST_BYTES));
		}

		@Test
		void cannotConvertNull() {
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> ByteArray.toBytes(null));
			assertThat(ex.getMessage(), containsString("cannot convert"));
		}

		@Test
		void cannotConvertInteger() {
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> ByteArray.toBytes(42));
			assertThat(ex.getMessage(), containsString("cannot convert"));
		}

		@Test
		void byteArrayInputReturnsDirectReference() {
			// Arrange:
			final byte[] input = TEST_BYTES.clone();

			// Act:
			final byte[] result = ByteArray.toBytes(input);

			// Assert:
			assertThat(result, sameInstance(input));
		}
	}

	// endregion
}
