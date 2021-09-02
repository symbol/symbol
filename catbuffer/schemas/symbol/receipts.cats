import "entity.cats"

# enumeration of receipt types
enum ReceiptType : uint16
	# reserved receipt type
	RESERVED = 0x0000

	# mosaic rental fee receipt type
	MOSAIC_RENTAL_FEE = 0x124D

	# namespace rental fee receipt type
	NAMESPACE_RENTAL_FEE = 0x134E

	# harvest fee receipt type
	HARVEST_FEE = 0x2143

	# lock hash completed receipt type
	LOCK_HASH_COMPLETED = 0x2248

	# lock hash expired receipt type
	LOCK_HASH_EXPIRED = 0x2348

	# lock secret completed receipt type
	LOCK_SECRET_COMPLETED = 0x2252

	# lock secret expired receipt type
	LOCK_SECRET_EXPIRED = 0x2352

	# lock hash created receipt type
	LOCK_HASH_CREATED = 0x3148

	# lock secret created receipt type
	LOCK_SECRET_CREATED = 0x3152

	# mosaic expired receipt type
	MOSAIC_EXPIRED = 0x414D

	# namespace expired receipt type
	NAMESPACE_EXPIRED = 0x414E

	# namespace deleted receipt type
	NAMESPACE_DELETED = 0x424E

	# inflation receipt type
	INFLATION = 0x5143

	# transaction group receipt type
	TRANSACTION_GROUP = 0xE143

	# address alias resolution receipt type
	ADDRESS_ALIAS_RESOLUTION = 0xF143

	# mosaic alias resolution receipt type
	MOSAIC_ALIAS_RESOLUTION = 0xF243

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

	# mosaic
	mosaic = Mosaic

	# mosaic sender address
	sender_address = Address

	# mosaic recipient address
	recipient_address = Address

# binary layout for a balance change receipt
struct BalanceChangeReceipt
	inline Receipt

	# mosaic
	mosaic = Mosaic

	# account address
	target_address = Address

# binary layout for an inflation receipt
struct InflationReceipt
	inline Receipt

	# mosaic
	mosaic = Mosaic

# binary layout for a mosaic expiry receipt
struct MosaicExpiryReceipt
	inline Receipt

	# expiring mosaic id
	artifact_id = MosaicId
