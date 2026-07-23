package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.nem.models.*;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link KeyPair} and {@link Verifier} for the NEM network against vectors from {@code tests/vectors/nem/crypto/1.test-keys.json}.
 */
final class KeyPairTest {
	private static final String DETERMINISTIC_PRIVATE_KEY = "ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57";
	private static final String EXPECTED_PUBLIC_KEY = "D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0";

	// Deterministic fixture key (first entry from tests/vectors/nem/crypto/1.test-keys.json).
	private static final String PRIVATE_KEY_HEX = "575DBB3062267EFF57C970A336EBBC8FBCFE12C5BD3ED7BC11EB0481D7704CED";

	@Test
	void canCreateKeyPairFromPrivateKey() {
		// Arrange:
		final CryptoTypes.PrivateKey privateKey = new CryptoTypes.PrivateKey(DETERMINISTIC_PRIVATE_KEY);

		// Act:
		final KeyPair keyPair = new KeyPair(privateKey);

		// Assert:
		assertThat(Converter.uint8ToHex(keyPair.getPublicKey().bytes()), equalTo(EXPECTED_PUBLIC_KEY));
		assertThat(Converter.uint8ToHex(keyPair.getPrivateKey().bytes()), equalTo(DETERMINISTIC_PRIVATE_KEY));
	}

	@Test
	void privateKeyIsNotMutatedByConstructor() {
		// Arrange:
		// NEM reverses the private-key bytes before passing to the underlying ed25519 impl;
		// ensure the original PrivateKey object is left untouched.
		final CryptoTypes.PrivateKey original = new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX);

		// Act:
		new KeyPair(original);

		// Assert:
		assertThat(Converter.uint8ToHex(original.bytes()), equalTo(PRIVATE_KEY_HEX));
	}

	@Test
	void signFillsSignature() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final byte[] message = {
				1, 2, 3, 4
		};

		// Act:
		final CryptoTypes.Signature signature = keyPair.sign(message);

		// Assert: a real signature is never all zeros.
		assertThat(Converter.uint8ToHex(signature.bytes()), not(equalTo(Converter.uint8ToHex(new byte[CryptoTypes.Signature.SIZE]))));
	}

	@Test
	void signaturesGeneratedForSameDataBySameKeyPairsAreEqual() {
		// Ed25519 signatures are deterministic: same key + same message -> same signature.
		// Arrange:
		final KeyPair kp1 = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));
		final KeyPair kp2 = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));
		final byte[] message = {
				1, 2, 3, 4
		};

		// Act:
		final CryptoTypes.Signature signature1 = kp1.sign(message);
		final CryptoTypes.Signature signature2 = kp2.sign(message);

		// Assert:
		assertThat(Converter.uint8ToHex(signature1.bytes()), equalTo(Converter.uint8ToHex(signature2.bytes())));
	}

	@Test
	void signaturesGeneratedForSameDataByDifferentKeyPairsAreDifferent() {
		// Arrange:
		final KeyPair kp1 = new KeyPair(CryptoTypes.PrivateKey.random());
		final KeyPair kp2 = new KeyPair(CryptoTypes.PrivateKey.random());
		final byte[] message = {
				1, 2, 3, 4
		};

		// Act:
		final CryptoTypes.Signature signature1 = kp1.sign(message);
		final CryptoTypes.Signature signature2 = kp2.sign(message);

		// Assert:
		assertThat(Converter.uint8ToHex(signature1.bytes()), not(equalTo(Converter.uint8ToHex(signature2.bytes()))));
	}

}
