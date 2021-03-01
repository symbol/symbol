import "transaction.cats"

# binary layout for a node key link transaction
struct NodeKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = Key

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded node key link transaction
struct NodeKeyLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424C

	inline Transaction
	inline NodeKeyLinkTransactionBody

# binary layout for an embedded node key link transaction
struct EmbeddedNodeKeyLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424C

	inline EmbeddedTransaction
	inline NodeKeyLinkTransactionBody
