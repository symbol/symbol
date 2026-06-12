from symbolchain.sc import Cosignature


def calculate_transaction_fee(transaction, fee_multiplier, cosignature_count=0):
	"""Calculates the minimum required transaction fee for a transaction."""

	return transaction.size * fee_multiplier + Cosignature().size * cosignature_count
