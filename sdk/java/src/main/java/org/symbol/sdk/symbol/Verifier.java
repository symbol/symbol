package org.symbol.sdk.symbol;

import java.util.Arrays;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.impl.Ed25519;
import org.symbol.sdk.impl.Tweetnacl;

/**
 * Verifies signatures signed by a single Symbol key pair.
 */
public final class Verifier {
	/** Public key used for signature verification. */
	public final CryptoTypes.PublicKey publicKey;

	/**
	 * Creates a verifier from a public key.
	 *
	 * @param publicKey Public key.
	 */
	public Verifier(final CryptoTypes.PublicKey publicKey) {
		if (Arrays.equals(new byte[CryptoTypes.PublicKey.SIZE], publicKey.bytes()))
			throw new IllegalArgumentException("public key cannot be zero");

		this.publicKey = publicKey;
	}

	/**
	 * Verifies a message signature.
	 *
	 * @param message Message to verify.
	 * @param signature Signature to verify.
	 * @return {@code true} if the message signature verifies.
	 */
	public boolean verify(final byte[] message, final CryptoTypes.Signature signature) {
		return Ed25519.verify(Tweetnacl.HashMode.SHA2_512, message, signature.bytes(), publicKey.bytes());
	}
}
