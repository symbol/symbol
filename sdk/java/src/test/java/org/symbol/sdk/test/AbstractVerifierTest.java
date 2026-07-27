package org.symbol.sdk.test;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.utils.Converter;

/**
 * Shared verifier contract tests run against both the Symbol and NEM verifiers.
 *
 * @param <TKeyPair> Concrete key pair type.
 * @param <TVerifier> Concrete verifier type.
 */
public abstract class AbstractVerifierTest<TKeyPair extends KeyPair, TVerifier> {
	/**
	 * @param privateKey Private key.
	 * @return Key pair created around the private key.
	 */
	protected abstract TKeyPair createKeyPair(CryptoTypes.PrivateKey privateKey);

	/**
	 * @param publicKey Public key.
	 * @return Verifier created around the public key.
	 */
	protected abstract TVerifier createVerifier(CryptoTypes.PublicKey publicKey);

	/**
	 * @param verifier Verifier.
	 * @param message Message to verify.
	 * @param signature Signature to verify.
	 * @return Whether the signature verifies.
	 */
	protected abstract boolean verify(TVerifier verifier, byte[] message, CryptoTypes.Signature signature);

	private static byte[] mutateBytes(final byte[] bytes, final int position) {
		final byte[] mutated = bytes.clone();
		mutated[position] ^= (byte) 0xFF;
		return mutated;
	}

	@Test
	void cannotCreateVerifierAroundZeroPublicKey() {
		// Arrange:
		final CryptoTypes.PublicKey zero = new CryptoTypes.PublicKey(new byte[CryptoTypes.PublicKey.SIZE]);

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> createVerifier(zero));

		// Assert:
		assertThat(ex.getMessage(), containsString("public key cannot be zero"));
	}

	@Test
	void canVerifySignature() {
		// Arrange:
		final byte[] message = AbstractKeyPairTest.randomMessage();
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		// Act:
		final boolean isVerified = verify(createVerifier(keyPair.getPublicKey()), message, signature);

		// Assert:
		assertThat(isVerified, is(true));
	}

	@Test
	void cannotVerifySignatureWithDifferentKeyPair() {
		// Arrange: random raw public-key bytes
		// rather than throwing
		final byte[] message = AbstractKeyPairTest.randomMessage();
		final CryptoTypes.Signature signature = createKeyPair(CryptoTypes.PrivateKey.random()).sign(message);
		final CryptoTypes.PublicKey randomPublicKey = new CryptoTypes.PublicKey(CryptoTypes.PrivateKey.random().bytes());

		// Act:
		final boolean isVerified = verify(createVerifier(randomPublicKey), message, signature);

		// Assert:
		assertThat(isVerified, is(false));
	}

	@Test
	void verifyReturnsFalseForEmptySignature() {
		// Arrange:
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());

		// Act:
		final boolean isVerified = verify(createVerifier(keyPair.getPublicKey()), new byte[]{
				1, 2, 3
		}, new CryptoTypes.Signature(AbstractKeyPairTest.randomMessage(CryptoTypes.Signature.SIZE)));

		// Assert:
		assertThat(isVerified, is(false));
	}

	@Test
	void cannotVerifySignatureWhenMessageIsModified() {
		// Arrange:
		final byte[] message = AbstractKeyPairTest.randomMessage();
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		final TVerifier verifier = createVerifier(keyPair.getPublicKey());
		for (int i = 0; i < message.length; ++i) {
			final byte[] modifiedMessage = mutateBytes(message, i);

			// Act:
			final boolean isVerified = verify(verifier, modifiedMessage, signature);

			// Assert:
			assertThat("modification at index " + i, isVerified, is(false));
		}
	}

	@Test
	void cannotVerifySignatureWhenSignatureIsModified() {
		// Arrange: (covers all 64 signature positions; the JS suite iterates only the first message.length of them)
		final byte[] message = AbstractKeyPairTest.randomMessage();
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		final TVerifier verifier = createVerifier(keyPair.getPublicKey());
		for (int i = 0; i < CryptoTypes.Signature.SIZE; ++i) {
			final CryptoTypes.Signature modifiedSignature = new CryptoTypes.Signature(mutateBytes(signature.bytes(), i));

			// Act:
			final boolean isVerified = verify(verifier, message, modifiedSignature);

			// Assert:
			assertThat("modification at index " + i, isVerified, is(false));
		}
	}

	@Test
	void cannotVerifySignatureWithZeroS() {
		// Arrange: keep R (first 32 bytes), zero S (last 32 bytes).
		final byte[] message = AbstractKeyPairTest.randomMessage();
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature signature = keyPair.sign(message);

		final byte[] zeroSBytes = signature.bytes().clone();
		Arrays.fill(zeroSBytes, CryptoTypes.Signature.SIZE / 2, CryptoTypes.Signature.SIZE, (byte) 0);
		final CryptoTypes.Signature signatureZeroS = new CryptoTypes.Signature(zeroSBytes);

		final TVerifier verifier = createVerifier(keyPair.getPublicKey());

		// Act:
		final boolean isVerified = verify(verifier, message, signature);
		final boolean isVerifiedZeroS = verify(verifier, message, signatureZeroS);

		// Assert:
		assertThat(isVerified, is(true));
		assertThat(isVerifiedZeroS, is(false));
	}

	// ed25519 group order L, little endian (2^252 + 27742317777372353535851937790883648493).
	private static final byte[] GROUP_ORDER_L = Converter.hexToUint8("EDD3F55C1A631258D69CF7A2DEF9DE1400000000000000000000000000000010");

	private static byte[] scalarAddGroupOrder(final byte[] scalar) {
		int remainder = 0;
		for (int i = 0; i < GROUP_ORDER_L.length; ++i) {
			final int byteSum = (scalar[i] & 0xFF) + (GROUP_ORDER_L[i] & 0xFF) + remainder;
			scalar[i] = (byte) byteSum;
			remainder = byteSum >>> 8;
		}

		return scalar;
	}

	@Test
	void cannotVerifyNonCanonicalSignature() {
		// Arrange:
		// the value 30 in the payload ensures that the encodedS part of the signature is < 2 ^ 253 after adding the group order
		final byte[] message = Converter.hexToUint8("0102030405060708091D");
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.Signature canonicalSignature = keyPair.sign(message);

		// keep R (first 32 bytes) and add the group order to S, like the JS scalarAddGroupOrder helper
		final byte[] nonCanonicalSignatureBytes = canonicalSignature.bytes().clone();
		System.arraycopy(scalarAddGroupOrder(Arrays.copyOfRange(nonCanonicalSignatureBytes, 32, 64)), 0, nonCanonicalSignatureBytes, 32,
				32);
		final CryptoTypes.Signature nonCanonicalSignature = new CryptoTypes.Signature(nonCanonicalSignatureBytes);

		final TVerifier verifier = createVerifier(keyPair.getPublicKey());

		// Act:
		final boolean isVerifiedCanonical = verify(verifier, message, canonicalSignature);
		final boolean isVerifiedNonCanonical = verify(verifier, message, nonCanonicalSignature);

		// Assert:
		assertThat(isVerifiedCanonical, is(true));
		assertThat(isVerifiedNonCanonical, is(false));
	}
}
