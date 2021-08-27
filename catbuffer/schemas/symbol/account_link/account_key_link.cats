import "transaction.cats"

# binary layout for an account key link transaction
struct AccountKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = PublicKey

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded account key link transaction
struct AccountKeyLinkTransaction
	const uint8 transaction_version = 1
	const TransactionType transaction_type = account_key_link

	inline Transaction
	inline AccountKeyLinkTransactionBody

# binary layout for an embedded account key link transaction
struct EmbeddedAccountKeyLinkTransaction
	const uint8 transaction_version = 1
	const TransactionType transaction_type = account_key_link

	inline EmbeddedTransaction
	inline AccountKeyLinkTransactionBody
