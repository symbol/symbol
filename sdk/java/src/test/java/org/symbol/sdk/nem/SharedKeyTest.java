package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link SharedKey}
 */
final class SharedKeyTest {
	@FunctionalInterface
	private interface SharedKeyDeriver {
		CryptoTypes.SharedKey256 derive(KeyPair keyPair, CryptoTypes.PublicKey otherPublicKey);
	}

	private static final byte[] DETERMINISTIC_SALT = "1234567890ABCDEF1234567890ABCDEF".getBytes(StandardCharsets.UTF_8);

	// region basic shared key suite

	abstract static class BasicSharedKeyTests {
		private final SharedKeyDeriver deriver;

		BasicSharedKeyTests(final SharedKeyDeriver deriver) {
			this.deriver = deriver;
		}

		private String deriveHex(final CryptoTypes.PrivateKey privateKey, final CryptoTypes.PublicKey otherPublicKey) {
			return Converter.uint8ToHex(deriver.derive(new KeyPair(privateKey), otherPublicKey).bytes());
		}

		@Test
		void sharedKeysGeneratedWithSameInputsAreEqual() {
			// Arrange:
			final CryptoTypes.PrivateKey privateKey = CryptoTypes.PrivateKey.random();
			final CryptoTypes.PublicKey otherPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();

			// Act:
			final String sharedKey1 = deriveHex(privateKey, otherPublicKey);
			final String sharedKey2 = deriveHex(privateKey, otherPublicKey);

			// Assert:
			assertThat(sharedKey2, equalTo(sharedKey1));
		}

		@Test
		void sharedKeysGeneratedForDifferentPrivateKeysAreDifferent() {
			// Arrange:
			final CryptoTypes.PrivateKey privateKey1 = CryptoTypes.PrivateKey.random();
			final byte[] privateKey2Bytes = privateKey1.bytes().clone();
			privateKey2Bytes[0] ^= (byte) 0xFF;
			final CryptoTypes.PublicKey otherPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();

			// Act:
			final String sharedKey1 = deriveHex(privateKey1, otherPublicKey);
			final String sharedKey2 = deriveHex(new CryptoTypes.PrivateKey(privateKey2Bytes), otherPublicKey);

			// Assert:
			assertThat(sharedKey2, not(equalTo(sharedKey1)));
		}

		@Test
		void sharedKeysGeneratedForDifferentOtherPublicKeysAreDifferent() {
			// Arrange:
			final CryptoTypes.PrivateKey privateKey = CryptoTypes.PrivateKey.random();
			final byte[] otherPublicKeyBytes = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey().bytes();

			// Act:
			final String baseline = deriveHex(privateKey, new CryptoTypes.PublicKey(otherPublicKeyBytes));

			// Act + Assert: retry across mutations because a mutated key may be rejected by the subgroup check.
			boolean checked = false;
			for (int i = 1; 256 > i; ++i) {
				final byte[] mutated = otherPublicKeyBytes.clone();
				mutated[0] ^= (byte) i;
				try {
					assertThat(deriveHex(privateKey, new CryptoTypes.PublicKey(mutated)), not(equalTo(baseline)));
					checked = true;
					break;
				} catch (final IllegalArgumentException ex) {
					// mutated key landed off the curve / outside the main subgroup; try another mutation
				}
			}

			assertThat(checked, equalTo(true));
		}

		@Test
		void mutualSharedResultsAreEqual() {
			// Arrange:
			final KeyPair keyPair1 = new KeyPair(CryptoTypes.PrivateKey.random());
			final KeyPair keyPair2 = new KeyPair(CryptoTypes.PrivateKey.random());

			// Act:
			final String sharedKey1 = Converter.uint8ToHex(deriver.derive(keyPair1, keyPair2.getPublicKey()).bytes());
			final String sharedKey2 = Converter.uint8ToHex(deriver.derive(keyPair2, keyPair1.getPublicKey()).bytes());

			// Assert:
			assertThat(sharedKey1, equalTo(sharedKey2));
		}

		@Test
		void publicKeyNotOnTheCurveThrows() {
			// Arrange:
			final KeyPair keyPair1 = new KeyPair(CryptoTypes.PrivateKey.random());
			final byte[] otherPublicKeyBytes = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey().bytes();

			// Act + Assert: retry because just setting a byte often still lands on a valid curve point.
			boolean threwInvalidPoint = false;
			for (int i = 0; 4 > i; ++i) {
				final byte[] invalidPublicKeyBytes = otherPublicKeyBytes.clone();
				invalidPublicKeyBytes[31] = (byte) i;
				try {
					deriver.derive(keyPair1, new CryptoTypes.PublicKey(invalidPublicKeyBytes));
				} catch (final IllegalArgumentException ex) {
					if (null != ex.getMessage() && ex.getMessage().contains("invalid point")) {
						threwInvalidPoint = true;
						break;
					}
				}
			}

			// Assert:
			assertThat(threwInvalidPoint, equalTo(true));
		}
	}

	@Nested
	final class DeriveSharedKey extends BasicSharedKeyTests {
		DeriveSharedKey() {
			super(SharedKey::deriveSharedKey);
		}
	}

	/** runs the basic suite with a deterministic salt. */
	@Nested
	final class DeriveSharedKeyDeprecated extends BasicSharedKeyTests {
		DeriveSharedKeyDeprecated() {
			super((keyPair, otherPublicKey) -> SharedKey.deriveSharedKeyDeprecated(keyPair, otherPublicKey, DETERMINISTIC_SALT));
		}

		@Test
		void saltWithInvalidLengthIsRejected() {
			// Arrange:
			final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final CryptoTypes.PublicKey otherPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
			final byte[][] invalidSalts = {
					new byte[31], Arrays.copyOf(DETERMINISTIC_SALT, 31), new byte[33]
			};

			// Act + Assert:
			for (final byte[] invalidSalt : invalidSalts) {
				final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
						() -> SharedKey.deriveSharedKeyDeprecated(keyPair, otherPublicKey, invalidSalt));
				assertThat(ex.getMessage(), containsString("invalid salt"));
			}
		}
	}

	// endregion
}
