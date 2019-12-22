import "receipts.cats"
import "resolution_statement/resolution_statement_types.cats"

# binary layout for a mosaic resolution statement
struct MosaicResolutionStatement
	inline Receipt

	# unresolved mosaic
	unresolved = UnresolvedMosaicId

	# resolution entries
	resolutionEntries = array(MosaicResolutionEntry, __FILL__)

# binary layout for an address resolution statement
struct AddressResolutionStatement
	inline Receipt

	# unresolved address
	unresolved = UnresolvedAddress

	# resolution entries
	resolutionEntries = array(AddressResolutionEntry, __FILL__)
