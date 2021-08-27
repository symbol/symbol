import "entity.cats"

# enumeration of receipt types
enum ReceiptType : uint16
	# reserved receipt type
	reserved = 0x0000

	# mosaic rental fee receipt type
	mosaic_rental_fee = 0x124D

	# namespace rental fee receipt type
	namespace_rental_fee = 0x134E

	# harvest fee receipt type
	harvest_fee = 0x2143

	# lock hash completed receipt type
	lockHash_completed = 0x2248

	# lock hash expired receipt type
	lockHash_expired = 0x2348

	# lock secret completed receipt type
	lockSecret_completed = 0x2252

	# lock secret expired receipt type
	lockSecret_expired = 0x2352

	# lock hash created receipt type
	lockHash_created = 0x3148

	# lock secret created receipt type
	lockSecret_created = 0x3152

	# mosaic expired receipt type
	mosaic_expired = 0x414D

	# namespace expired receipt type
	namespace_expired = 0x414E

	# namespace deleted receipt type
	namespace_deleted = 0x424E

	# inflation receipt type
	inflation = 0x5143

	# transaction group receipt type
	transaction_group = 0xE143

	# address alias resolution receipt type
	address_alias_resolution = 0xF143

	# mosaic alias resolution receipt type
	mosaic_alias_resolution = 0xF243

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
	senderAddress = Address

	# mosaic recipient address
	recipientAddress = Address

# binary layout for a balance change receipt
struct BalanceChangeReceipt
	inline Receipt

	# mosaic
	mosaic = Mosaic

	# account address
	targetAddress = Address

# binary layout for an inflation receipt
struct InflationReceipt
	inline Receipt

	# mosaic
	mosaic = Mosaic

# binary layout for a mosaic expiry receipt
struct MosaicExpiryReceipt
	inline Receipt

	# expiring mosaic id
	artifactId = MosaicId
