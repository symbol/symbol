# binary layout for receipt source
struct ReceiptSource
	# transaction primary source (e.g. index within block)
	primary_id = uint32

	# transaction secondary source (e.g. index within aggregate)
	secondary_id = uint32

# binary layout for address resolution entry
struct AddressResolutionEntry
	# source of resolution within block
	source = ReceiptSource

	# resolved value
	resolved = Address

# binary layout for mosaic resolution entry
struct MosaicResolutionEntry
	# source of resolution within block
	source = ReceiptSource

	# resolved value
	resolved = MosaicId
