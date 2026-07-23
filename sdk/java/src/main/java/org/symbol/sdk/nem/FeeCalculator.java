package org.symbol.sdk.nem;

import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;

import org.symbol.sdk.nem.models.Message;
import org.symbol.sdk.nem.models.MosaicId;
import org.symbol.sdk.nem.models.SizePrefixedMosaic;
import org.symbol.sdk.nem.models.Transaction;
import org.symbol.sdk.nem.models.TransactionType;
import org.symbol.sdk.nem.models.TransferTransactionV1;
import org.symbol.sdk.nem.models.TransferTransactionV2;

/**
 * Calculates minimum required NEM fees.
 */
public final class FeeCalculator {
	private static final BigInteger XEM_SUPPLY = BigInteger.valueOf(8_999_999_999L);

	private static final BigInteger MAX_MOSAIC_UNITS = BigInteger.valueOf(9_000_000_000_000_000L);

	private static final BigInteger XEM_TRANSFER_FEE_STEP = BigInteger.valueOf(10_000L);

	private static final BigInteger MAX_XEM_TRANSFER_FEE = BigInteger.valueOf(25L);

	private static final long FEE_UNIT = 50_000L;

	private FeeCalculator() {
	}

	/** Mosaic information required for fee calculation. */
	public record MosaicInformation(long supply, int divisibility) {
	}

	/** Fully qualified mosaic name passed to a mosaic information lookup. */
	public record MosaicName(String namespaceName, String name) {
		@Override
		public String toString() {
			return namespaceName + ":" + name;
		}
	}

	/**
	 * Calculates the minimum required mosaic rental fee.
	 *
	 * @return Rental fee.
	 */
	public static long calculateMosaicRentalFee() {
		return 10L * 1_000_000L;
	}

	/**
	 * Calculates the minimum required namespace rental fee.
	 *
	 * @param isRoot true if the fee should be calculated for a root namespace.
	 * @return Rental fee.
	 */
	public static long calculateNamespaceRentalFee(final boolean isRoot) {
		return (isRoot ? 100L : 10L) * 1_000_000L;
	}

	/**
	 * Calculates the minimum required transaction fee for a transaction. Without a mosaic information lookup, fees for custom mosaic
	 * transfers cannot be calculated and are rejected.
	 *
	 * @param transaction Transaction.
	 * @return Transaction fee.
	 */
	public static long calculateTransactionFee(final Transaction transaction) {
		return calculateTransactionFee(transaction, mosaicName -> null);
	}

	/**
	 * Calculates the minimum required transaction fee for a transaction.
	 *
	 * @param transaction Transaction.
	 * @param mosaicInformationMap Mosaic information indexed by fully qualified mosaic name ({@code "namespace:name"}).
	 * @return Transaction fee.
	 */
	public static long calculateTransactionFee(final Transaction transaction, final Map<String, MosaicInformation> mosaicInformationMap) {
		return calculateTransactionFee(transaction, mosaicName -> mosaicInformationMap.get(mosaicName.toString()));
	}

	/**
	 * Calculates the minimum required transaction fee for a transaction.
	 *
	 * @param transaction Transaction.
	 * @param mosaicInformationLookup Looks up mosaic information given a fully qualified mosaic name; {@code null} means unknown.
	 * @return Transaction fee.
	 */
	public static long calculateTransactionFee(final Transaction transaction,
			final Function<MosaicName, MosaicInformation> mosaicInformationLookup) {
		if (TransactionType.TRANSFER != transaction.getType())
			return weightWithFeeUnit(TransactionType.MULTISIG_ACCOUNT_MODIFICATION == transaction.getType() ? 10L : 3L);

		return weightWithFeeUnit(calculateUnweightedTransferFee(transaction, mosaicInformationLookup));
	}

	private static long weightWithFeeUnit(final long amount) {
		return amount * FEE_UNIT;
	}

	private static long calculateUnweightedTransferFee(final Transaction transaction,
			final Function<MosaicName, MosaicInformation> mosaicInformationLookup) {
		final long amountMicroXem;
		final Optional<Message> message;
		final List<SizePrefixedMosaic> mosaics;
		if (transaction instanceof TransferTransactionV2 transfer) {
			amountMicroXem = transfer.getAmount().value();
			message = transfer.getMessage();
			mosaics = transfer.getMosaics();
		} else if (transaction instanceof TransferTransactionV1 transfer) {
			amountMicroXem = transfer.getAmount().value();
			message = transfer.getMessage();
			mosaics = List.of();
		} else {
			throw new IllegalArgumentException("unexpected TRANSFER transaction " + transaction.getClass().getSimpleName());
		}

		final long messageFee = message.map(value -> value.getMessage().length / 32 + 1L).orElse(0L);
		final long amount = amountMicroXem / 1_000_000L; // convert to XEM whole units
		if (mosaics.isEmpty())
			return messageFee + calculateXemTransferFee(BigInteger.valueOf(amount));

		long transferFee = 0L;
		for (final SizePrefixedMosaic mosaic : mosaics) {
			final MosaicName mosaicName = decodeMosaicId(mosaic.getMosaic().getMosaicId());
			final MosaicInformation mosaicInformation = mosaicInformationLookup.apply(mosaicName);
			if (null == mosaicInformation)
				throw new IllegalArgumentException("unable to find fee information for " + mosaicName);

			transferFee += calculateMosaicTransferFee(amount, mosaic, mosaicInformation);
		}

		return messageFee + transferFee;
	}

	private static MosaicName decodeMosaicId(final MosaicId mosaicId) {
		return new MosaicName(new String(mosaicId.getNamespaceId().getName(), StandardCharsets.UTF_8),
				new String(mosaicId.getName(), StandardCharsets.UTF_8));
	}

	private static long calculateXemTransferFee(final BigInteger amount) {
		return amount.divide(XEM_TRANSFER_FEE_STEP).max(BigInteger.ONE).min(MAX_XEM_TRANSFER_FEE).longValueExact();
	}

	private static BigInteger calculateMosaicTotalQuantity(final MosaicInformation mosaicInformation) {
		return BigInteger.valueOf(mosaicInformation.supply()).multiply(BigInteger.TEN.pow(mosaicInformation.divisibility()));
	}

	private static BigInteger calculateXemEquivalent(final long amount, final long mosaicAmount,
			final MosaicInformation mosaicInformation) {
		if (0L == mosaicInformation.supply())
			return BigInteger.ZERO;

		// amount XEM whole units
		// mosaicAmount mosaic atomic units
		// XEM_SUPPLY / calculateMosaicTotalQuantity(mosaicInformation) convert mosaicAmount from mosaic units to XEM equivalent units
		return BigInteger.valueOf(amount).multiply(BigInteger.valueOf(mosaicAmount)).multiply(XEM_SUPPLY)
				.divide(calculateMosaicTotalQuantity(mosaicInformation));
	}

	private static long calculateMosaicTransferFee(final long amount, final SizePrefixedMosaic mosaic,
			final MosaicInformation mosaicInformation) {
		if (0 == mosaicInformation.divisibility() && 10_000L >= mosaicInformation.supply())
			return 1L;

		final long xemFee = calculateXemTransferFee(
				calculateXemEquivalent(amount, mosaic.getMosaic().getAmount().value(), mosaicInformation));
		final BigInteger mosaicTotalQuantity = calculateMosaicTotalQuantity(mosaicInformation);
		// no mosaic can have more atomic units than MAX_MOSAIC_UNITS
		if (0 < mosaicTotalQuantity.compareTo(MAX_MOSAIC_UNITS))
			throw new IllegalArgumentException("mosaic total quantity " + mosaicTotalQuantity + " exceeds " + MAX_MOSAIC_UNITS);

		final long supplyRelatedAdjustment = 0 < mosaicTotalQuantity.signum()
				? (long) (0.8 * Math.log(MAX_MOSAIC_UNITS.divide(mosaicTotalQuantity).doubleValue()))
				: 0L;
		return Math.max(1L, xemFee - supplyRelatedAdjustment);
	}
}
