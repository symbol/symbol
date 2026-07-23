package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link KeyPair} and {@link Verifier}; the exhaustive cross-SDK vector coverage runs via {@code ./gradlew vectors} (AllVectors).
 */
final class KeyPairTest {
	// Deterministic fixture key (first entry from tests/vectors/symbol/crypto/1.test-keys.json).
	private static final String PRIVATE_KEY_HEX = "575DBB3062267EFF57C970A336EBBC8FBCFE12C5BD3ED7BC11EB0481D7704CED";

	@Test
	void canCreateKeyPairFromPrivateKey() {
		// Arrange:
		final String expectedPublicKey = "E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD";
		final String privateKey = "E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E";

		// Act:
		final KeyPair keyPair = new KeyPair(new CryptoTypes.PrivateKey(privateKey));

		// Assert:
		assertThat(Converter.uint8ToHex(keyPair.getPublicKey().bytes()), equalTo(expectedPublicKey));
		assertThat(Converter.uint8ToHex(keyPair.getPrivateKey().bytes()), equalTo(privateKey));
	}

	@Test
	void privateKeyAccessorReturnsCopy() {
		// Arrange:
		final CryptoTypes.PrivateKey original = new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX);
		final KeyPair kp = new KeyPair(original);

		// Act: access the private key, mutate the returned bytes, then access again
		final CryptoTypes.PrivateKey accessed = kp.getPrivateKey();
		final String accessedHex = Converter.uint8ToHex(accessed.bytes());
		accessed.bytes()[0] ^= (byte) 0xFF;
		final String reaccessedHex = Converter.uint8ToHex(kp.getPrivateKey().bytes());

		// Assert: mutating the returned bytes must not affect the keypair's internal state
		assertThat(accessedHex, equalTo(PRIVATE_KEY_HEX));
		assertThat(reaccessedHex, equalTo(PRIVATE_KEY_HEX));
	}

	@Test
	void signaturesGeneratedForSameDataBySameKeyPairsAreEqual() {
		// Ed25519 signatures are deterministic: same key + same message -> same signature.
		// Arrange:
		final KeyPair kp1 = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));
		final KeyPair kp2 = new KeyPair(new CryptoTypes.PrivateKey(PRIVATE_KEY_HEX));
		final byte[] message = new byte[]{
				1, 2, 3, 4
		};

		// Act:
		final CryptoTypes.Signature signature1 = kp1.sign(message);
		final CryptoTypes.Signature signature2 = kp2.sign(message);

		// Assert:
		assertThat(Converter.uint8ToHex(signature1.bytes()), equalTo(Converter.uint8ToHex(signature2.bytes())));
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
