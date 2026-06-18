package org.symbol.sdk.impl;

import java.util.Arrays;

/**
 * High-level Ed25519 keypair derivation, signing and verification with a swappable hash so the same primitive serves both SHA-512 (Symbol)
 * and Keccak-512 (NEM).
 */
public final class Ed25519 {
	private Ed25519() {
	}

	/**
	 * Derives a keypair from a 32-byte seed using the chosen hash mode.
	 *
	 * @param mode Hash mode (Sha2_512 / Keccak).
	 * @param seed Seed bytes (length must be 32).
	 * @return Resulting keypair.
	 */
	public static Tweetnacl.KeyPair keyPairFromSeed(final Tweetnacl.HashMode mode, final byte[] seed) {
		return Tweetnacl.signKeyPairFromSeed(seed, mode);
	}

	/**
	 * Computes a detached signature over {@code message} with {@code keyPair}.
	 *
	 * @param mode Hash mode.
	 * @param message Message bytes.
	 * @param keyPair Signing keypair.
	 * @return 64-byte detached signature.
	 */
	public static byte[] sign(final Tweetnacl.HashMode mode, final byte[] message, final Tweetnacl.KeyPair keyPair) {
		return Tweetnacl.signDetached(message, keyPair.secretKey, mode);
	}

	/**
	 * Verifies a detached signature over {@code message} against {@code publicKey} and additionally enforces canonicality of the {@code S}
	 * component.
	 *
	 * @param mode Hash mode.
	 * @param message Message bytes.
	 * @param signature 64-byte detached signature.
	 * @param publicKey 32-byte public key.
	 * @return {@code true} if the signature is valid <em>and</em> {@code S} is canonical.
	 */
	public static boolean verify(final Tweetnacl.HashMode mode, final byte[] message, final byte[] signature, final byte[] publicKey) {
		if (!Tweetnacl.signDetachedVerify(message, signature, publicKey, mode))
			return false;

		final byte[] encodedS = new byte[32];
		System.arraycopy(signature, 32, encodedS, 0, 32);
		return isCanonicalS(encodedS);
	}

	private static boolean isCanonicalS(final byte[] encodedS) {
		// require canonical signature
		final double[] x = new double[64];
		for (int i = 0; i < 32; ++i)
			x[i] = encodedS[i] & 0xFF;
		// rest is zero

		final byte[] reducedEncodedS = new byte[64];
		Tweetnacl.modL(reducedEncodedS, 0, x);

		final byte[] reducedFirst32 = new byte[32];
		System.arraycopy(reducedEncodedS, 0, reducedFirst32, 0, 32);
		return Arrays.equals(encodedS, reducedFirst32);
	}
}
