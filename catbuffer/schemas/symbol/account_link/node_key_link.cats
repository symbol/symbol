import "transaction.cats"

# binary layout for a node key link transaction
struct NodeKeyLinkTransactionBody
	# linked public key
	linked_public_key = PublicKey

	# link action
	link_action = LinkAction

# binary layout for a non-embedded node key link transaction
struct NodeKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NODE_KEY_LINK)

	inline Transaction
	inline NodeKeyLinkTransactionBody

# binary layout for an embedded node key link transaction
struct EmbeddedNodeKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NODE_KEY_LINK)

	inline EmbeddedTransaction
	inline NodeKeyLinkTransactionBody
