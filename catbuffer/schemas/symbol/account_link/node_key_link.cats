import "transaction.cats"

# binary layout for a node key link transaction
struct NodeKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = PublicKey

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded node key link transaction
struct NodeKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, node_key_link)

	inline Transaction
	inline NodeKeyLinkTransactionBody

# binary layout for an embedded node key link transaction
struct EmbeddedNodeKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, node_key_link)

	inline EmbeddedTransaction
	inline NodeKeyLinkTransactionBody
