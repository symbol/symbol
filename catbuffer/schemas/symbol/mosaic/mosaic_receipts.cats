import "receipts.cats"

# Receipt generated when a mosaic expires.
struct MosaicExpiredReceipt
	RECEIPT_TYPE = make_const(ReceiptType, MOSAIC_EXPIRED)

	inline Receipt

	# Expiring mosaic id.
	artifact_id = MosaicId


# Receipt generated when a mosaic rental fee is paid.
struct MosaicRentalFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, MOSAIC_RENTAL_FEE)

	inline BalanceTransferReceipt
