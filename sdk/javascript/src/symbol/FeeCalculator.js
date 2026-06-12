import * as sc from './models.js';

/**
 * Calculates the minimum required transaction fee for a transaction.
 * @param {sc.Transaction} transaction Transaction.
 * @param {number} feeMultiplier Fee multiplier to use.
 * @param {number} cosignatureCount Number of expected cosignatures to be attached.
 * @returns {bigint} Transaction fee.
 */
const calculateTransactionFee = (transaction, feeMultiplier, cosignatureCount = 0) =>
	(BigInt(transaction.size) * BigInt(feeMultiplier)) + (BigInt(new sc.Cosignature().size) * BigInt(cosignatureCount));

export { calculateTransactionFee }; // eslint-disable-line import/prefer-default-export
