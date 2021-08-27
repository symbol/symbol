import "entity.cats"

# enumeration of transaction types
enum TransactionType : uint16
	# reserved transaction type
	reserved = 0x0000

# binary layout for a transaction
struct Transaction
	inline SizePrefixedEntity
	inline VerifiableEntity
	inline EntityBody

	# transaction type
	type = TransactionType

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp

# binary layout for an embedded transaction header
struct EmbeddedTransactionHeader
	inline SizePrefixedEntity

	# reserved padding to align end of EmbeddedTransactionHeader on 8-byte boundary
	embeddedTransactionHeader_Reserved1 = uint32

# binary layout for an embedded transaction
struct EmbeddedTransaction
	inline EmbeddedTransactionHeader
	inline EntityBody

	# transaction type
	type = TransactionType
