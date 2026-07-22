package org.symbol.sdk.symbol;

import org.symbol.sdk.Serializer;
import org.symbol.sdk.symbol.models.NetworkType;
import org.symbol.sdk.symbol.models.PublicKey;
import org.symbol.sdk.symbol.models.TransactionType;

/**
 * Common transaction surface implemented by the generated {@code Transaction} and {@code EmbeddedTransaction} roots — the transaction-only
 * layer above {@link org.symbol.sdk.CatbufferType}, consumed by the transaction factory and its post-processors instead of the all-types
 * CatbufferType. Not sealed only because the implementing roots live in the generated {@code models} package (an unnamed-module sealed
 * hierarchy cannot span packages); no other type should implement this.
 */
public interface BaseTransaction extends Serializer {
	/**
	 * Gets the transaction type.
	 *
	 * @return Transaction type.
	 */
	TransactionType getType();

	/**
	 * Gets the transaction version.
	 *
	 * @return Transaction version.
	 */
	int getVersion();

	/**
	 * Gets the transaction network.
	 *
	 * @return Transaction network.
	 */
	NetworkType getNetwork();

	/**
	 * Gets the signer public key.
	 *
	 * @return Signer public key.
	 */
	PublicKey getSignerPublicKey();

	/**
	 * Reads a named field. Used by the descriptor pipeline.
	 *
	 * @param name Field name.
	 * @return Field value.
	 */
	Object getField(String name);

	/**
	 * Sets a named field. Used by the rule-based descriptor pipeline.
	 *
	 * @param name Field name (descriptor key).
	 * @param value Value to assign.
	 */
	void setField(String name, Object value);

	/** Sorts the collections of this object so that it can be serialized canonically. */
	void sort();
}
