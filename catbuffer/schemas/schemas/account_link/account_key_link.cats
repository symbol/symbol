import "transaction.cats"

# binary layout for an account key link transaction
struct AccountKeyLinkTransactionBody
	# remote public key
	remotePublicKey = Key

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded account key link transaction
struct AccountKeyLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x414C

	inline Transaction
	inline AccountKeyLinkTransactionBody

# binary layout for an embedded account key link transaction
struct EmbeddedAccountKeyLinkTransaction
	inline EmbeddedTransaction
	inline AccountKeyLinkTransactionBody
