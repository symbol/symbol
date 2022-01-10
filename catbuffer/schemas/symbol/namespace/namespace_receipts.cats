import "namespace/namespace_types.cats"
import "receipts.cats"

# Receipt generated when a [namespace](/concepts/namespace.html) expires.
struct NamespaceExpiredReceipt
	RECEIPT_TYPE = make_const(ReceiptType, NAMESPACE_EXPIRED)

	inline Receipt

	# Expired namespace identifier.
	artifact_id = NamespaceId

# Receipt generated when a [namespace](/concepts/namespace.html) is deleted.
struct NamespaceDeletedReceipt
	RECEIPT_TYPE = make_const(ReceiptType, NAMESPACE_DELETED)

	inline Receipt

	# Deleted namespace identifier.
	artifact_id = NamespaceId

# Receipt generated when a namespace rental fee is paid.
struct NamespaceRentalFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, NAMESPACE_RENTAL_FEE)

	inline BalanceTransferReceipt
