import "entity.cats"
import "transaction_type.cats"

# binary layout for a transaction
@size(size)
@initializes(version, TRANSACTION_VERSION)
@initializes(type, TRANSACTION_TYPE)
@discriminator(type)
@is_aligned
abstract struct Transaction
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
inline struct EmbeddedTransactionHeader
	inline SizePrefixedEntity

	# reserved padding to align end of EmbeddedTransactionHeader on 8-byte boundary
	embedded_transaction_header_reserved_1 = make_reserved(uint32, 0)

# binary layout for an embedded transaction
@size(size)
@initializes(version, TRANSACTION_VERSION)
@initializes(type, TRANSACTION_TYPE)
@discriminator(type)
@is_aligned
abstract struct EmbeddedTransaction
	inline EmbeddedTransactionHeader
	inline EntityBody

	# transaction type
	type = TransactionType
