package org.symbol.sdk;

/**
 * Cross-blockchain key-pair contract implemented by the Symbol and NEM key pairs (which differ only in the Ed25519 hash mode and
 * private-key byte order). Each key pair signs with its own chain's algorithm.
 */
public interface KeyPair {
	/**
	 * Gets the public key.
	 *
	 * @return Public key.
	 */
	CryptoTypes.PublicKey getPublicKey();

	/**
	 * Gets the private key.
	 *
	 * @return Private key.
	 */
	CryptoTypes.PrivateKey getPrivateKey();

	/**
	 * Signs a message with the private key.
	 *
	 * @param message Message to sign.
	 * @return Message signature.
	 */
	CryptoTypes.Signature sign(byte[] message);
}
