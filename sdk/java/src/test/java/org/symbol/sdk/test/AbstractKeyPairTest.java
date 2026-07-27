package org.symbol.sdk.test;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;

import java.security.SecureRandom;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.utils.Converter;

/**
 * Shared key pair contract tests run against both the Symbol and NEM key pairs
 *
 * @param <TKeyPair> Concrete key pair type.
 */
public abstract class AbstractKeyPairTest<TKeyPair extends KeyPair> {
	private static final SecureRandom RANDOM = new SecureRandom();

	/**
	 * @param privateKey Private key.
	 * @return Key pair created around the private key.
	 */
	protected abstract TKeyPair createKeyPair(CryptoTypes.PrivateKey privateKey);

	/** @return Deterministic private key pinned by the chain's KeyPair. */
	protected abstract String deterministicPrivateKey();

	/** @return Public key the deterministic private key must derive to. */
	protected abstract String expectedPublicKey();

	static byte[] randomMessage() {
		return randomMessage(21);
	}

	static byte[] randomMessage(final int length) {
		final byte[] bytes = new byte[length];
		RANDOM.nextBytes(bytes);
		return bytes;
	}

	// region create

	@Test
	void canCreateKeyPairFromPrivateKey() {
		// Arrange:
		final String expectedPublicKey = expectedPublicKey();
		final CryptoTypes.PrivateKey privateKey = new CryptoTypes.PrivateKey(deterministicPrivateKey());

		// Act:
		final TKeyPair keyPair = createKeyPair(privateKey);

		// Assert:
		assertThat(Converter.uint8ToHex(keyPair.getPublicKey().bytes()), equalTo(expectedPublicKey));
		assertThat(Converter.uint8ToHex(keyPair.getPrivateKey().bytes()), equalTo(deterministicPrivateKey()));
	}

	// endregion

	// region sign

	@Test
	void signFillsSignature() {
		// Arrange:
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final byte[] message = randomMessage();

		// Act:
		final CryptoTypes.Signature signature = keyPair.sign(message);

		// Assert: a real signature is never all zeros.
		assertThat(Converter.uint8ToHex(signature.bytes()), not(equalTo(Converter.uint8ToHex(new byte[CryptoTypes.Signature.SIZE]))));
	}

	@Test
	void signaturesGeneratedForSameDataBySameKeyPairsAreEqual() {
		// Ed25519 signatures are deterministic: same key + same message -> same signature.
		// Arrange:
		final CryptoTypes.PrivateKey privateKey = CryptoTypes.PrivateKey.random();
		final TKeyPair keyPair1 = createKeyPair(privateKey);
		final TKeyPair keyPair2 = createKeyPair(privateKey);
		final byte[] message = randomMessage();

		// Act:
		final CryptoTypes.Signature signature1 = keyPair1.sign(message);
		final CryptoTypes.Signature signature2 = keyPair2.sign(message);

		// Assert:
		assertThat(Converter.uint8ToHex(signature2.bytes()), equalTo(Converter.uint8ToHex(signature1.bytes())));
	}

	@Test
	void signaturesGeneratedForSameDataByDifferentKeyPairsAreDifferent() {
		// Arrange:
		final TKeyPair keyPair1 = createKeyPair(CryptoTypes.PrivateKey.random());
		final TKeyPair keyPair2 = createKeyPair(CryptoTypes.PrivateKey.random());
		final byte[] message = randomMessage();

		// Act:
		final CryptoTypes.Signature signature1 = keyPair1.sign(message);
		final CryptoTypes.Signature signature2 = keyPair2.sign(message);

		// Assert:
		assertThat(Converter.uint8ToHex(signature2.bytes()), not(equalTo(Converter.uint8ToHex(signature1.bytes()))));
	}

	// endregion
}
