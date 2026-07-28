package org.symbol.sdk.nem;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.test.AbstractVerifierTest;

/**
 * Tests {@link Verifier} the shared verifier contract runs via {@link AbstractVerifierTest}.
 */
final class VerifierTest extends AbstractVerifierTest<KeyPair, Verifier> {
	@Override
	protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
		return new KeyPair(privateKey);
	}

	@Override
	protected Verifier createVerifier(final CryptoTypes.PublicKey publicKey) {
		return new Verifier(publicKey);
	}

	@Override
	protected boolean verify(final Verifier verifier, final byte[] message, final CryptoTypes.Signature signature) {
		return verifier.verify(message, signature);
	}
}
