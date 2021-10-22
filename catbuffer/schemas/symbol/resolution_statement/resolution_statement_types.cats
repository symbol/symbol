# The transaction inside the block that triggered the receipt.
struct ReceiptSource
	# Transaction primary source (e.g. index within the block).
	primary_id = uint32

	# Transaction secondary source (e.g. index within aggregate).
	secondary_id = uint32

# Actual Address behind a NamespaceId at the time a transaction was confirmed.
struct AddressResolutionEntry
	# Information about the transaction that triggered the receipt.
	source = ReceiptSource

	# Resolved Address.
	resolved = Address

# Actual MosaicId behind a NamespaceId at the time a transaction was confirmed.
struct MosaicResolutionEntry
	# Information about the transaction that triggered the receipt.
	source = ReceiptSource

	# Resolved MosaicId.
	resolved = MosaicId
