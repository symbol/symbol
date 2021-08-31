import "transaction.cats"

# binary layout for an account key link transaction
struct AccountKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = PublicKey

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded account key link transaction
struct AccountKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_key_link)

	inline Transaction
	inline AccountKeyLinkTransactionBody

# binary layout for an embedded account key link transaction
struct EmbeddedAccountKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_key_link)

	inline EmbeddedTransaction
	inline AccountKeyLinkTransactionBody
