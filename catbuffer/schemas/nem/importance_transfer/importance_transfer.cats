import "transaction.cats"

# enumeration of importance transfer modes
enum ImportanceTransferMode : uint32
	# activates an importance transfer
	LINK = 0x01

	# deactivates an importance transfer
	UNLINK = 0x02

# binary layout for an importance transfer transaction
struct ImportanceTransferTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, IMPORTANCE_TRANSFER)

	inline Transaction

	# importance transfer mode
	mode = ImportanceTransferMode

	# remote account public key size
	remote_public_key_size = make_reserved(uint32, 32)

	# public key of remote account to which importance should be transferred
	remote_public_key = PublicKey
