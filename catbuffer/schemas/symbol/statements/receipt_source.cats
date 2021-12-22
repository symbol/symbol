# The transaction inside the block that triggered the receipt.
struct ReceiptSource
	# Transaction primary source (e.g. index within the block).
	primary_id = uint32

	# Transaction secondary source (e.g. index within aggregate).
	secondary_id = uint32

