package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.security.SecureRandom;
import java.util.Arrays;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.models.*;

/**
 * Tests {@link Verifier} (Symbol / Ed25519 over SHA-512) independently of {@link KeyPair}: input validation, signature shape checks, that
 * the public key is stored without copying, and the per-index tamper loops.
 */
final class VerifierTest {
	// First vector from tests/vectors/symbol/crypto/1.test-keys.json.
	private static final String PRIVATE_KEY_HEX = "575DBB3062267EFF57C970A336EBBC8FBCFE12C5BD3ED7BC11EB0481D7704CED";

	private static byte[] randomMessage() {
		final byte[] message = new byte[21];
		new SecureRandom().nextBytes(message);
		return message;
	}

	private static byte[] mutateBytes(final byte[] bytes, final int position) {
		final byte[] mutated = bytes.clone();
		mutated[position] ^= (byte) 0xFF;
		return mutated;
	}

	@Test
	void canCreateFromNonZeroPublicKey() {
		// Arrange:
		final CryptoTypes.PublicKey publicKey = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX)).getPublicKey();

		// Act:
		final Verifier verifier = new Verifier(publicKey);

		// Assert:
		// the verifier exposes the key it was created with, by reference (no defensive copy).
		assertThat(verifier.publicKey, sameInstance(publicKey));
	}

	@Test
	void cannotCreateVerifierAroundZeroPublicKey() {
		// Arrange:
		final CryptoTypes.PublicKey zero = new CryptoTypes.PublicKey(new byte[CryptoTypes.PublicKey.SIZE]);

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> new Verifier(zero));

		// Assert:
		assertThat(ex.getMessage(), containsString("public key cannot be zero"));
	}

	@Test
	void canVerifySignature() {
		// Arrange:
		final KeyPair kp = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));
		final byte[] message = "verify me".getBytes();
		final CryptoTypes.Signature signature = kp.sign(message);

		// Act:
		final boolean actual = new Verifier(kp.getPublicKey()).verify(message, signature);

		// Assert:
		assertThat(actual, is(true));
	}

	@Test
	void cannotVerifySignatureWithDifferentKeyPair() {
		// Arrange:
		final KeyPair signer = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));
		final KeyPair other = new KeyPair(new CryptoTypes.PrivateKey("5B0E3FA5D3B49A79022D7C1E121BA1CBBF4DB5821F47AB8C708EF88DEFC29BFE"));
		final byte[] message = "verify me".getBytes();
		final CryptoTypes.Signature signature = signer.sign(message);

		// Act:
		final boolean actual = new Verifier(other.getPublicKey()).verify(message, signature);

		// Assert:
		assertThat(actual, is(false));
	}

	@Test
	void verifyReturnsFalseForEmptySignature() {
		// Arrange:
		final KeyPair kp = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));

		// Act:
		final boolean actual = new Verifier(kp.getPublicKey()).verify(new byte[]{
				1, 2, 3
		}, new CryptoTypes.Signature(new byte[CryptoTypes.Signature.SIZE]));

		// Assert:
		assertThat(actual, is(false));
	}

	@Test
	void cannotVerifySignatureWhenMessageIsModified() {
		// Arrange:
		final byte[] message = randomMessage();
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		final Verifier verifier = new Verifier(keyPair.getPublicKey());
		for (int i = 0; i < message.length; ++i) {
			final byte[] modifiedMessage = mutateBytes(message, i);

			// Act:
			final boolean isVerified = verifier.verify(modifiedMessage, signature);

			// Assert:
			assertThat("modification at index " + i, isVerified, is(false));
		}
	}

	@Test
	void cannotVerifySignatureWhenSignatureIsModified() {
		// Arrange:
		final byte[] message = randomMessage();
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		final Verifier verifier = new Verifier(keyPair.getPublicKey());
		for (int i = 0; i < CryptoTypes.Signature.SIZE; ++i) {
			final CryptoTypes.Signature modifiedSignature = new CryptoTypes.Signature(mutateBytes(signature.bytes(), i));

			// Act:
			final boolean isVerified = verifier.verify(message, modifiedSignature);

			// Assert:
			assertThat("modification at index " + i, isVerified, is(false));
		}
	}

	@Test
	void cannotVerifySignatureWithZeroS() {
		// Arrange:
		final byte[] message = randomMessage();
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		final byte[] zeroSBytes = signature.bytes().clone();
		Arrays.fill(zeroSBytes, 32, CryptoTypes.Signature.SIZE, (byte) 0);
		final CryptoTypes.Signature signatureZeroS = new CryptoTypes.Signature(zeroSBytes);

		final Verifier verifier = new Verifier(keyPair.getPublicKey());

		// Act:
		final boolean isVerified = verifier.verify(message, signature);
		final boolean isVerifiedZeroS = verifier.verify(message, signatureZeroS);

		// Assert:
		assertThat(isVerified, is(true));
		assertThat(isVerifiedZeroS, is(false));
	}
}
