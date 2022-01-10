import "receipts.cats"

# Receipt generated when secret lock is created.
struct LockSecretCreatedFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, LOCK_SECRET_CREATED)

	inline BalanceChangeReceipt

# Receipt generated when secret lock is completed.
struct LockSecretCompletedFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, LOCK_SECRET_COMPLETED)

	inline BalanceChangeReceipt

# Receipt generated when secret lock is expired.
struct LockSecretExpiredFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, LOCK_SECRET_EXPIRED)

	inline BalanceChangeReceipt
