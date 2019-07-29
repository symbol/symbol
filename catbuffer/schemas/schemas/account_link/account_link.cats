import "account_link/account_link_types.cats"
import "transaction.cats"

# binary layout for an account link transaction
struct AccountLinkTransactionBody
	# remote account key
	remoteAccountKey = Key

	# account link action
	linkAction = AccountLinkAction

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
