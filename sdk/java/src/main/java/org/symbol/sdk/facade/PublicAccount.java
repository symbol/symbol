package org.symbol.sdk.facade;

import org.symbol.sdk.ByteArray;
import org.symbol.sdk.CryptoTypes;

/**
 * Cross-blockchain public account — a known public key plus its network-derived address. Implemented by the per-chain facade account types,
 * whose overrides narrow {@link #address()} to the concrete chain address.
 */
public interface PublicAccount {
	/**
	 * Gets the account public key.
	 *
	 * @return Public key.
	 */
	CryptoTypes.PublicKey publicKey();

	/**
	 * Gets the account address.
	 *
	 * @return Address.
	 */
	ByteArray address();
}
