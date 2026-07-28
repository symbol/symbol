package org.symbol.sdk.nem;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.impl.SharedKeyHelpers;
import org.symbol.sdk.impl.Tweetnacl;
import org.symbol.sdk.utils.ArrayHelpers;
import org.symbol.sdk.utils.Transforms;

/**
 * NEM shared-key derivation entry point. Uses HKDF-SHA-256 with the {@code "nem-nis1"} info tag and the NEM (Keccak-512) hash mode; private
 * key bytes are reversed before being passed to the derivation.
 */
public final class SharedKey {
	private static final SharedKeyHelpers.SharedSecretDeriver DERIVE_SECRET = SharedKeyHelpers
			.deriveSharedSecretFactory(Tweetnacl.HashMode.KECCAK_512);

	private static final SharedKeyHelpers.SharedKeyDeriver DERIVE = SharedKeyHelpers.deriveSharedKeyFactory("nem-nis1",
			Tweetnacl.HashMode.KECCAK_512);

	private SharedKey() {
	}

	/**
	 * Derives a shared key from a key pair and the other party's public key.
	 *
	 * @param keyPair Key pair.
	 * @param otherPublicKey Other party's public key.
	 * @return Shared encryption key.
	 */
	public static CryptoTypes.SharedKey256 deriveSharedKey(final KeyPair keyPair, final CryptoTypes.PublicKey otherPublicKey) {
		return DERIVE.derive(ArrayHelpers.reverse(keyPair.getPrivateKey().bytes()), otherPublicKey);
	}

	/**
	 * Derives a shared key using the legacy NEM scheme; provided only for compatibility with messages encrypted by original NEM wallets —
	 * new code should use {@link #deriveSharedKey(KeyPair, CryptoTypes.PublicKey)}.
	 *
	 * @param keyPair Key pair.
	 * @param otherPublicKey Other party's public key.
	 * @param salt Random salt; should be unique per use. Must be 32 bytes long.
	 * @return Shared encryption key.
	 */
	public static CryptoTypes.SharedKey256 deriveSharedKeyDeprecated(final KeyPair keyPair, final CryptoTypes.PublicKey otherPublicKey,
			final byte[] salt) {
		if (CryptoTypes.SharedKey256.SIZE != salt.length)
			throw new IllegalArgumentException("invalid salt");

		final byte[] reversed = ArrayHelpers.reverse(keyPair.getPrivateKey().bytes());
		final byte[] sharedSecret = DERIVE_SECRET.derive(reversed, otherPublicKey);
		final byte[] sharedKeyBytes = new byte[CryptoTypes.SharedKey256.SIZE];
		for (int i = 0; i < CryptoTypes.SharedKey256.SIZE; ++i)
			sharedKeyBytes[i] = (byte) (sharedSecret[i] ^ salt[i]);

		return new CryptoTypes.SharedKey256(Transforms.keccak_256(sharedKeyBytes));
	}
}
