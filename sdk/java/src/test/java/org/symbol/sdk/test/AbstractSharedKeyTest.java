package org.symbol.sdk.test;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;

import java.util.function.BiConsumer;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.utils.Converter;

/**
 * Shared shared-key contract tests run against both the Symbol and NEM derivations.
 *
 * @param <TKeyPair> Concrete key pair type.
 */
public abstract class AbstractSharedKeyTest<TKeyPair extends KeyPair> {
	/**
	 * @param privateKey Private key.
	 * @return Key pair created around the private key.
	 */
	protected abstract TKeyPair createKeyPair(CryptoTypes.PrivateKey privateKey);

	/**
	 * @param keyPair Key pair.
	 * @param otherPublicKey Other party's public key.
	 * @return Shared encryption key.
	 */
	protected abstract CryptoTypes.SharedKey256 deriveSharedKey(TKeyPair keyPair, CryptoTypes.PublicKey otherPublicKey);

	private String deriveHex(final CryptoTypes.PrivateKey privateKey, final CryptoTypes.PublicKey otherPublicKey) {
		return Converter.uint8ToHex(deriveSharedKey(createKeyPair(privateKey), otherPublicKey).bytes());
	}

	private CryptoTypes.PublicKey randomPublicKey() {
		return createKeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
	}

	private void assertDerivedSharedResult(final BiConsumer<byte[], byte[]> mutate, final BiConsumer<String, String> assertion) {
		// Arrange:
		final CryptoTypes.PrivateKey privateKey1 = CryptoTypes.PrivateKey.random();
		final CryptoTypes.PublicKey otherPublicKey1 = randomPublicKey();

		final byte[] privateKey2Bytes = privateKey1.bytes().clone();
		final byte[] otherPublicKey2Bytes = otherPublicKey1.bytes().clone();
		mutate.accept(privateKey2Bytes, otherPublicKey2Bytes);

		// Act:
		final String sharedKey1 = deriveHex(privateKey1, otherPublicKey1);
		final String sharedKey2 = deriveHex(new CryptoTypes.PrivateKey(privateKey2Bytes), new CryptoTypes.PublicKey(otherPublicKey2Bytes));

		// Assert:
		assertion.accept(sharedKey1, sharedKey2);
	}

	@Test
	void sharedKeysGeneratedWithSameInputsAreEqual() {
		assertDerivedSharedResult((privateKeyBytes, otherPublicKeyBytes) -> {
		}, (lhs, rhs) -> assertThat(rhs, equalTo(lhs)));
	}

	@Test
	void sharedKeysGeneratedForDifferentPrivateKeysAreDifferent() {
		assertDerivedSharedResult((privateKeyBytes, otherPublicKeyBytes) -> privateKeyBytes[0] ^= (byte) 0xFF,
				(lhs, rhs) -> assertThat(rhs, not(equalTo(lhs))));
	}

	@Test
	void sharedKeysGeneratedForDifferentOtherPublicKeysAreDifferent() {
		// retry across mutations because a mutated key may be rejected by the subgroup check
		boolean checked = false;
		for (int i = 1; 256 > i; ++i) {
			final int mask = i;
			try {
				assertDerivedSharedResult((privateKeyBytes, otherPublicKeyBytes) -> otherPublicKeyBytes[0] ^= (byte) mask,
						(lhs, rhs) -> assertThat(rhs, not(equalTo(lhs))));
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
		final TKeyPair keyPair1 = createKeyPair(CryptoTypes.PrivateKey.random());
		final TKeyPair keyPair2 = createKeyPair(CryptoTypes.PrivateKey.random());

		// Act:
		final String sharedKey1 = Converter.uint8ToHex(deriveSharedKey(keyPair1, keyPair2.getPublicKey()).bytes());
		final String sharedKey2 = Converter.uint8ToHex(deriveSharedKey(keyPair2, keyPair1.getPublicKey()).bytes());

		// Assert:
		assertThat(sharedKey1, equalTo(sharedKey2));
	}

	@Test
	void publicKeyNotOnTheCurveThrows() {
		// Arrange:
		final TKeyPair keyPair1 = createKeyPair(CryptoTypes.PrivateKey.random());
		final byte[] otherPublicKeyBytes = randomPublicKey().bytes();

		// Act: retry because just setting a byte often still lands on a valid curve point.
		boolean threwInvalidPoint = false;
		for (int i = 0; 4 > i; ++i) {
			final byte[] invalidPublicKeyBytes = otherPublicKeyBytes.clone();
			invalidPublicKeyBytes[31] = (byte) i;
			try {
				deriveSharedKey(keyPair1, new CryptoTypes.PublicKey(invalidPublicKeyBytes));
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
