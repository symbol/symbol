import "receipts.cats"

# Receipt generated when hash lock is created.
struct LockHashCreatedFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, LOCK_HASH_CREATED)

	inline BalanceChangeReceipt

# Receipt generated when hash lock is completed.
struct LockHashCompletedFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, LOCK_HASH_COMPLETED)

	inline BalanceChangeReceipt

# Receipt generated when hash lock is expired.
struct LockHashExpiredFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, LOCK_HASH_EXPIRED)

	inline BalanceChangeReceipt
