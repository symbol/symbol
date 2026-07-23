package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.nullValue;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.symbol.models.*;

/**
 * Tests {@link Metadata}.
 */
final class MetadataTest {
	@Nested
	class GenerateKey {
		@Test
		void setsHighBit() {
			// Act:
			final long key = Metadata.generateKey("hello");

			// Assert: the high bit (63) is forced to one, so the key is negative as a signed long.
			assertThat(key < 0L, is(true));
		}

		@Test
		void canGenerateExpectedKeysFromSeeds() {
			// Act:
			final long keyA = Metadata.generateKey("a");
			final long keyAbc = Metadata.generateKey("abc");
			final long keyDef = Metadata.generateKey("def");

			// Assert:
			assertThat(keyA, equalTo(0xF524A0FBF24B0880L));
			assertThat(keyAbc, equalTo(0xB225E24FA75D983AL));
			assertThat(keyDef, equalTo(0xB0AC5222678F0D8EL));
		}

		@Test
		void isDeterministic() {
			// Act:
			final long key1 = Metadata.generateKey("foo");
			final long key2 = Metadata.generateKey("foo");

			// Assert:
			assertThat(key1, equalTo(key2));
		}

		@Test
		void isDifferentForDifferentSeeds() {
			// Act:
			final long fooKey = Metadata.generateKey("foo");
			final long barKey = Metadata.generateKey("bar");

			// Assert:
			assertThat(fooKey, not(equalTo(barKey)));
		}
	}

	@Nested
	class UpdateValue {
		private static final byte[] OLD_VALUE = {
				(byte) 0x9A, (byte) 0xC7, 0x33, 0x18, (byte) 0xA7, (byte) 0xB0, 0x36
		};

		@Test
		void canSetNewValueWithoutOldValue() {
			// Arrange:
			final byte[] newValue = OLD_VALUE.clone();

			// Act:
			final byte[] result = Metadata.updateValue(null, newValue);

			// Assert:
			assertThat(result, equalTo(newValue));
			result[0] = 99;
			assertThat(newValue[0], is((byte) 0x9A));
		}

		@Test
		void canUpdateEqualLengthValue() {
			// Arrange:
			final byte[] newValue = {
					(byte) 0xD4, 0x60, (byte) 0x82, (byte) 0xF8, 0x78, (byte) 0xFE, 0x78
			};

			// Act:
			final byte[] result = Metadata.updateValue(OLD_VALUE, newValue);

			// Assert:
			assertThat(result, equalTo(new byte[]{
					(byte) (0x9A ^ 0xD4), (byte) (0xC7 ^ 0x60), (byte) (0x33 ^ 0x82), (byte) (0x18 ^ 0xF8), (byte) (0xA7 ^ 0x78),
					(byte) (0xB0 ^ 0xFE), (byte) (0x36 ^ 0x78)
			}));
		}

		@Test
		void canUpdateShorterValue() {
			// Arrange:
			final byte[] newValue = {
					(byte) 0xD4, 0x60, (byte) 0x82, (byte) 0xF8
			};

			// Act:
			final byte[] result = Metadata.updateValue(OLD_VALUE, newValue);

			// Assert:
			assertThat(result, equalTo(new byte[]{
					(byte) (0x9A ^ 0xD4), (byte) (0xC7 ^ 0x60), (byte) (0x33 ^ 0x82), (byte) (0x18 ^ 0xF8), (byte) 0xA7, (byte) 0xB0, 0x36
			}));
		}

		@Test
		void canUpdateLongerValue() {
			// Arrange:
			final byte[] newValue = {
					(byte) 0xD4, 0x60, (byte) 0x82, (byte) 0xF8, 0x78, (byte) 0xFE, 0x78, (byte) 0xE6, (byte) 0x9D, (byte) 0xD6
			};

			// Act:
			final byte[] result = Metadata.updateValue(OLD_VALUE, newValue);

			// Assert:
			assertThat(result, equalTo(new byte[]{
					(byte) (0x9A ^ 0xD4), (byte) (0xC7 ^ 0x60), (byte) (0x33 ^ 0x82), (byte) (0x18 ^ 0xF8), (byte) (0xA7 ^ 0x78),
					(byte) (0xB0 ^ 0xFE), (byte) (0x36 ^ 0x78), (byte) 0xE6, (byte) 0x9D, (byte) 0xD6
			}));
		}

		@Test
		void neverReturnsNull() {
			// Act:
			final byte[] actual = Metadata.updateValue(new byte[]{}, new byte[]{});

			// Assert:
			assertThat(actual, is(not(nullValue())));
		}
	}
}
