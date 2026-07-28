package org.symbol.sdk.symbol;

import org.symbol.sdk.symbol.models.Cosignature;
import org.symbol.sdk.symbol.models.Transaction;

/**
 * Calculates minimum required Symbol transaction fees.
 */
public final class FeeCalculator {
	private static final long COSIGNATURE_SIZE = new Cosignature().size();

	private FeeCalculator() {
	}

	/**
	 * Calculates the minimum required transaction fee for a transaction.
	 *
	 * @param transaction Transaction.
	 * @param feeMultiplier Fee multiplier to use.
	 * @return Transaction fee.
	 */
	public static long calculateTransactionFee(final Transaction transaction, final long feeMultiplier) {
		return calculateTransactionFee(transaction, feeMultiplier, 0);
	}

	/**
	 * Calculates the minimum required transaction fee for a transaction.
	 *
	 * @param transaction Transaction.
	 * @param feeMultiplier Fee multiplier to use.
	 * @param cosignatureCount Number of expected cosignatures to be attached.
	 * @return Transaction fee.
	 */
	public static long calculateTransactionFee(final Transaction transaction, final long feeMultiplier, final int cosignatureCount) {
		return (long) transaction.size() * feeMultiplier + COSIGNATURE_SIZE * cosignatureCount;
	}
}
