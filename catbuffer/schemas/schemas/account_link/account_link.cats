import "transaction.cats"

# binary layout for an account link transaction
struct AccountLinkTransactionBody
	# remote public key
	remotePublicKey = Key

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded account link transaction
struct AccountLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x414C

	inline Transaction
	inline AccountLinkTransactionBody

# binary layout for an embedded account link transaction
struct EmbeddedAccountLinkTransaction
	inline EmbeddedTransaction
	inline AccountLinkTransactionBody
