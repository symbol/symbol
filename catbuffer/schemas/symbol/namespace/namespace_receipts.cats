import "namespace/namespace_types.cats"
import "receipts.cats"

# Receipt generated when a [namespace](/concepts/namespace.html) expires.
struct NamespaceExpiryReceipt
	inline Receipt

	# Expiring namespace identifier.
	artifact_id = NamespaceId
