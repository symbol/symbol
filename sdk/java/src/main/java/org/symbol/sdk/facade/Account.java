package org.symbol.sdk.facade;

import org.symbol.sdk.CatbufferType;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;

/**
 * Cross-blockchain account — a {@link PublicAccount} that also holds the signing key pair and can sign its own chain's transactions.
 *
 * @param <TTransaction> Concrete transaction type.
 * @param <TKeyPair> Concrete key-pair type.
 */
public interface Account<TTransaction extends CatbufferType, TKeyPair extends KeyPair> extends PublicAccount {
	/**
	 * Gets the account key pair.
	 *
	 * @return Key pair.
	 */
	TKeyPair keyPair();

	/**
	 * Signs a transaction with this account's key pair.
	 *
	 * @param transaction Transaction.
	 * @return Signature.
	 */
	CryptoTypes.Signature signTransaction(TTransaction transaction);
}
