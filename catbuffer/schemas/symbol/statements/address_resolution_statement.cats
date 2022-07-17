import "statements/receipt_source.cats"
import "types.cats"

# Actual Address behind a NamespaceId at the time a transaction was confirmed.
struct AddressResolutionEntry
	# Information about the transaction that triggered the receipt.
	source = ReceiptSource

	# Resolved Address.
	resolved_value = Address

# An Address resolution statement links a namespace alias used in a transaction to the real address
# **at the time of the transaction**.
struct AddressResolutionStatement
	# Unresolved address.
	unresolved = UnresolvedAddress

	# Number of resolution entries.
	resolution_entries_count = uint32

	# Resolution entries.
	resolution_entries = array(AddressResolutionEntry, resolution_entries_count)
