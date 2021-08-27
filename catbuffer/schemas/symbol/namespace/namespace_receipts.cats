import "namespace/namespace_types.cats"
import "receipts.cats"

# binary layout for a namespace expiry receipt
struct NamespaceExpiryReceipt
	inline Receipt

	# expiring namespace id
	artifactId = NamespaceId
