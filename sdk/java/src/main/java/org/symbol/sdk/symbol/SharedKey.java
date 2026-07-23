package org.symbol.sdk.symbol;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.impl.SharedKeyHelpers;
import org.symbol.sdk.impl.Tweetnacl;

/**
 * Symbol shared-key derivation: HKDF-SHA-256 with the {@code "catapult"} info tag and the Symbol (SHA-512) hash mode.
 */
public final class SharedKey {
	private static final SharedKeyHelpers.SharedKeyDeriver DERIVE = SharedKeyHelpers.deriveSharedKeyFactory("catapult",
			Tweetnacl.HashMode.SHA2_512);

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
		return DERIVE.derive(keyPair.getPrivateKey().bytes(), otherPublicKey);
	}
}
