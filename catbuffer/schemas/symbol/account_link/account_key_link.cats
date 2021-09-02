import "transaction.cats"

# binary layout for an account key link transaction
struct AccountKeyLinkTransactionBody
	# linked public key
	linked_public_key = PublicKey

	# link action
	link_action = LinkAction

# binary layout for a non-embedded account key link transaction
struct AccountKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_KEY_LINK)

	inline Transaction
	inline AccountKeyLinkTransactionBody

# binary layout for an embedded account key link transaction
struct EmbeddedAccountKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_KEY_LINK)

	inline EmbeddedTransaction
	inline AccountKeyLinkTransactionBody
