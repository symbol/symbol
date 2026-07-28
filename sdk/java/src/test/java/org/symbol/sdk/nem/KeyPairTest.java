package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.test.AbstractKeyPairTest;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link KeyPair} and {@link Verifier} for the NEM network against vectors from {@code tests/vectors/nem/crypto/1.test-keys.json}.
 */
final class KeyPairTest extends AbstractKeyPairTest<KeyPair> {
	@Override
	protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
		return new KeyPair(privateKey);
	}

	@Override
	protected String deterministicPrivateKey() {
		return "ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57";
	}

	@Override
	protected String expectedPublicKey() {
		return "D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0";
	}

	@Test
	void privateKeyIsNotMutatedByConstructor() {
		// Arrange:
		// NEM reverses the private-key bytes before passing to the underlying ed25519 impl;
		// ensure the original PrivateKey object is left untouched.
		final String privateKey = Converter.uint8ToHex(CryptoTypes.PrivateKey.random().bytes());
		final CryptoTypes.PrivateKey original = new CryptoTypes.PrivateKey(privateKey);

		// Act:
		new KeyPair(original);

		// Assert:
		assertThat(Converter.uint8ToHex(original.bytes()), equalTo(privateKey));
	}
}
