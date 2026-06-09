import * as nc from './models.js';

/**
 * Calculates the minimum required mosaic rental fee.
 * @returns {bigint} Rental fee.
 */
const calculateMosaicRentalFee = () => 10n * 1_000_000n;

/**
 * Calculates the minimum required namespace rental fee.
 * @param {boolean} isRoot \c true if the fee should be calculated for a root namespace.
 * @returns {bigint} Rental fee.
 */
const calculateNamespaceRentalFee = isRoot => ((isRoot ? 100n : 10n) * 1_000_000n);

const decodeMosaicId = mosaicId => {
	const decoder = new TextDecoder();
	return { namespaceId: { name: decoder.decode(mosaicId.namespaceId.name) }, name: decoder.decode(mosaicId.name) };
};

const calculateUnweightedTransferFee = (transaction, mosaicInformationLookup) => {
	const XEM_SUPPLY = 8_999_999_999n;
	const MAX_MOSAIC_UNITS = 9_000_000_000_000_000n;

	// Math.min and Math.max don't work with bigint
	const min = (lhs, rhs) => (lhs < rhs ? lhs : rhs);
	const max = (lhs, rhs) => (lhs > rhs ? lhs : rhs);

	const calculateXemTransferFee = amount => min(25n, max(1n, amount / 10000n));

	const calculateMosaicTotalQuantity = mosaicInformation => mosaicInformation.supply * (10n ** BigInt(mosaicInformation.divisibility));

	const calculateXemEquivalent = (amount, /** @type {bigint} */ mosaicAmount, mosaicInformation) => {
		if (0n === mosaicInformation.supply)
			return 0n;

		// amount          XEM whole units
		// mosaicAmount    mosaic atomic units
		// XEM_SUPPLY / calculateMosaicTotalQuantity(mosaicInformation)    convert mosaicAmount from mosaic units to XEM equivalent units
		return amount * mosaicAmount * XEM_SUPPLY / calculateMosaicTotalQuantity(mosaicInformation);
	};

	const calculateMosaicTransferFee = (amount, mosaic, mosaicInformation) => {
		if (0 === mosaicInformation.divisibility && 10_000n >= mosaicInformation.supply)
			return 1n;

		const xemEquivalent = calculateXemEquivalent(amount, mosaic.mosaic.amount.value, mosaicInformation);
		const xemFee = calculateXemTransferFee(xemEquivalent);
		const mosaicTotalQuantity = calculateMosaicTotalQuantity(mosaicInformation);
		const supplyRelatedAdjustment = 0 < mosaicTotalQuantity
			? BigInt(Math.trunc(0.8 * Math.log(Number(MAX_MOSAIC_UNITS / mosaicTotalQuantity))))
			: 0n;
		return max(1n, xemFee - supplyRelatedAdjustment);
	};

	const messageFee = BigInt(!transaction.message ? 0 : Math.trunc(transaction.message.message.length / 32) + 1);
	const amount = transaction.amount.value / 1_000_000n; // convert to XEM whole units
	if (0 === transaction.mosaics.length) {
		const transferFee = calculateXemTransferFee(amount);
		return messageFee + transferFee;
	}

	const transferFee = transaction.mosaics.reduce((totalFee, mosaic) => {
		const mosaicId = decodeMosaicId(mosaic.mosaic.mosaicId);
		const mosaicInformation = mosaicInformationLookup(mosaicId);
		if (!mosaicInformation)
			throw Error(`unable to find fee information for ${mosaicId.namespaceId.name}:${mosaicId.name}`);

		return totalFee + calculateMosaicTransferFee(amount, mosaic, mosaicInformation);
	}, 0n);

	return messageFee + transferFee;
};

/**
 * Calculates the minimum required transaction fee for a transaction.
 * @param {nc.Transaction} transaction Transaction.
 * @param {Function|{[key: string]: object}|undefined} mosaicInformationLookup
 * Looks up mosaic information ({supply, divisibility}) given mosaic identifier.
 * When a function, mosaic identifier will be passed as parameter.
 * When an object map, fully qualified mosaic identifier will be used as an index.
 * When undefined, this function will be unable to calculate fees for custom mosaic transfers.
 * @returns {bigint} Transaction fee.
 */
const calculateTransactionFee = (transaction, mosaicInformationLookup = undefined) => {
	const weightWithFeeUnit = amount => amount * 50_000n;

	if (nc.TransactionType.TRANSFER !== transaction.type)
		return weightWithFeeUnit(nc.TransactionType.MULTISIG_ACCOUNT_MODIFICATION === transaction.type ? 10n : 3n);

	const makeLookupFunction = lookup => ('function' === typeof lookup
		? lookup
		: mosaicId => lookup[`${mosaicId.namespaceId.name}:${mosaicId.name}`]);
	return weightWithFeeUnit(calculateUnweightedTransferFee(transaction, makeLookupFunction(mosaicInformationLookup)));
};

export {
	calculateMosaicRentalFee,
	calculateNamespaceRentalFee,
	calculateTransactionFee
};
