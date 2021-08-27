import "transaction.cats"

# enumeration of importance transfer modes
enum ImportanceTransferMode : uint32
	# activates an importance transfer
	link = 0x01

	# deactivates an importance transfer
	unlink = 0x02

# binary layout for an importance transfer transaction
struct ImportanceTransferTransaction
	version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, importance_transfer)

	inline Transaction

	# importance transfer mode
	mode = ImportanceTransferMode

	# remote account public key size
	remotePublicKeySize = make_reserved(uint32, 32)

	# public key of remote account to which importance should be transferred
	remotePublicKey = PublicKey
