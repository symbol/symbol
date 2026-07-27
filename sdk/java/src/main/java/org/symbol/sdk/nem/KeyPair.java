package org.symbol.sdk.nem;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.impl.Ed25519;
import org.symbol.sdk.impl.Tweetnacl;
import org.symbol.sdk.utils.ArrayHelpers;

/**
 * Represents an Ed25519 private and public key for the NEM network. NEM uses Keccak-512 as the Ed25519 hash function and reverses the
 * private key bytes before passing them to the underlying Ed25519 implementation.
 */
public final class KeyPair implements org.symbol.sdk.KeyPair {
	private static final Tweetnacl.HashMode HASH_MODE = Tweetnacl.HashMode.KECCAK_512;

	private final CryptoTypes.PrivateKey privateKey;
	private final Tweetnacl.KeyPair keyPair;

	/**
	 * Creates a key pair from a private key.
	 *
	 * @param privateKey Private key.
	 */
	public KeyPair(final CryptoTypes.PrivateKey privateKey) {
		this.privateKey = new CryptoTypes.PrivateKey(privateKey.bytes());
		this.keyPair = Ed25519.keyPairFromSeed(HASH_MODE, ArrayHelpers.reverse(privateKey.bytes()));
	}

	/**
	 * Gets the public key.
	 *
	 * @return Public key.
	 */
	public CryptoTypes.PublicKey getPublicKey() {
		return new CryptoTypes.PublicKey(keyPair.publicKey);
	}

	/**
	 * Gets the private key.
	 *
	 * @return Private key.
	 */
	public CryptoTypes.PrivateKey getPrivateKey() {
		return new CryptoTypes.PrivateKey(privateKey.bytes());
	}

	/**
	 * Signs a message with the private key.
	 *
	 * @param message Message to sign.
	 * @return Message signature.
	 */
	public CryptoTypes.Signature sign(final byte[] message) {
		return new CryptoTypes.Signature(Ed25519.sign(HASH_MODE, message, keyPair));
	}
}
