import "statements/receipt_source.cats"
import "types.cats"

# Actual MosaicId behind a NamespaceId at the time a transaction was confirmed.
struct MosaicResolutionEntry
	# Information about the transaction that triggered the receipt.
	source = ReceiptSource

	# Resolved MosaicId.
	resolved_value = MosaicId

# A Mosaic resolution statement links a namespace alias used in a transaction to the real mosaic id
# **at the time of the transaction**.
struct MosaicResolutionStatement
	# Unresolved mosaic.
	unresolved = UnresolvedMosaicId

	# Number of resolution entries.
	resolution_entries_count = uint32

	# Resolution entries.
	resolution_entries = array(MosaicResolutionEntry, resolution_entries_count)
