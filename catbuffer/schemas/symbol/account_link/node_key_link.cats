import "transaction.cats"

# binary layout for a node key link transaction
struct NodeKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = PublicKey

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded node key link transaction
struct NodeKeyLinkTransaction
	const uint8 transaction_version = 1
	const TransactionType transaction_type = node_key_link

	inline Transaction
	inline NodeKeyLinkTransactionBody

# binary layout for an embedded node key link transaction
struct EmbeddedNodeKeyLinkTransaction
	const uint8 transaction_version = 1
	const TransactionType transaction_type = node_key_link

	inline EmbeddedTransaction
	inline NodeKeyLinkTransactionBody
