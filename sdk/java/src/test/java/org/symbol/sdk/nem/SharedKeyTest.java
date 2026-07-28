package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.test.AbstractSharedKeyTest;

/**
 * Tests {@link SharedKey}: the shared contract runs via {@link AbstractSharedKeyTest} per variant.
 */
final class SharedKeyTest {

	private static final byte[] DETERMINISTIC_SALT = "1234567890ABCDEF1234567890ABCDEF".getBytes(StandardCharsets.UTF_8);

	private abstract class BasicSharedKeyTest extends AbstractSharedKeyTest<KeyPair> {
		@Override
		protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
			return new KeyPair(privateKey);
		}
	}

	@Nested
	final class DeriveSharedKey extends BasicSharedKeyTest {
		@Override
		protected CryptoTypes.SharedKey256 deriveSharedKey(final KeyPair keyPair, final CryptoTypes.PublicKey otherPublicKey) {
			return SharedKey.deriveSharedKey(keyPair, otherPublicKey);
		}
	}

	/** runs the basic suite with a deterministic salt. */
	@Nested
	final class DeriveSharedKeyDeprecated extends BasicSharedKeyTest {
		@Override
		protected CryptoTypes.SharedKey256 deriveSharedKey(final KeyPair keyPair, final CryptoTypes.PublicKey otherPublicKey) {
			return SharedKey.deriveSharedKeyDeprecated(keyPair, otherPublicKey, DETERMINISTIC_SALT);
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
}
