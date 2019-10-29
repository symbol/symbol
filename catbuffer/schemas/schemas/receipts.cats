import "entity.cats"

# enumeration of receipt types
enum ReceiptType : uint16
	# reserved receipt type
	reserved = 0x0000

# binary layout for a receipt entity
struct Receipt
	inline SizePrefixedEntity

	# receipt version
	version = uint16

	# receipt type
	type = ReceiptType

# binary layout for a balance transfer receipt
struct BalanceTransferReceipt
	inline Receipt

	# mosaic id
	mosaicId = MosaicId

	# amount
	amount = Amount

	# mosaic sender public key
	senderPublicKey = Key

	# mosaic recipient address
	recipientAddress = Address

# binary layout for a balance change receipt
struct BalanceChangeReceipt
	inline Receipt

	# mosaic id
	mosaicId = MosaicId

	# amount
	amount = Amount

	# account public key
	targetPublicKey = Key

# binary layout for an inflation receipt
struct InflationReceipt
	inline Receipt

	# mosaic id
	mosaicId = MosaicId

	# amount
	amount = Amount

# binary layout for a mosaic expiry receipt
struct MosaicExpiryReceipt
	inline Receipt

	# expiring mosaic id
	artifactId = MosaicId
