package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.sameInstance;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.test.AbstractKeyPairTest;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link KeyPair}; the shared key pair contract runs via {@link AbstractKeyPairTest}.
 */
final class KeyPairTest extends AbstractKeyPairTest<KeyPair> {
	@Override
	protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
		return new KeyPair(privateKey);
	}

	@Override
	protected String deterministicPrivateKey() {
		return "E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E";
	}

	@Override
	protected String expectedPublicKey() {
		return "E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD";
	}

	@Test
	void privateKeyAccessorReturnsCopy() {
		// Arrange:
		final CryptoTypes.PrivateKey original = CryptoTypes.PrivateKey.random();
		final KeyPair kp = new KeyPair(original);

		// Act: access the private key, mutate the returned bytes, then access again
		final CryptoTypes.PrivateKey accessed = kp.getPrivateKey();
		final String accessedHex = Converter.uint8ToHex(accessed.bytes());
		accessed.bytes()[0] ^= (byte) 0xFF;
		final String reaccessedHex = Converter.uint8ToHex(kp.getPrivateKey().bytes());

		// Assert: mutating the returned bytes must not affect the keypair's internal state
		assertThat(accessed, not(sameInstance(original)));
		assertThat(accessedHex, equalTo(Converter.uint8ToHex(original.bytes())));
		assertThat(reaccessedHex, equalTo(Converter.uint8ToHex(original.bytes())));
	}
}
