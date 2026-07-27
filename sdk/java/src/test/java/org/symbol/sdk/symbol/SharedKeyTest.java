package org.symbol.sdk.symbol;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.test.AbstractSharedKeyTest;

/**
 * Tests {@link SharedKey}: the shared contract runs via {@link AbstractSharedKeyTest}.
 */
final class SharedKeyTest extends AbstractSharedKeyTest<KeyPair> {
	@Override
	protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
		return new KeyPair(privateKey);
	}

	@Override
	protected CryptoTypes.SharedKey256 deriveSharedKey(final KeyPair keyPair, final CryptoTypes.PublicKey otherPublicKey) {
		return SharedKey.deriveSharedKey(keyPair, otherPublicKey);
	}
}
