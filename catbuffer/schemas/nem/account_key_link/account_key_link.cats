import "transaction.cats"

# enumeration of link actions
enum LinkAction : uint32
	# unlink account
	LINK = 0x01

	# link account
	UNLINK = 0x02

# shared content between verifiable and non-verifiable account key link transactions
inline struct AccountKeyLinkTransactionBody
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_KEY_LINK)

	# link action
	link_action = LinkAction

	# [__value__] public key of remote account to which importance should be transferred
	#
	# [size] remote account public key size
	remote_public_key = inline SizePrefixedPublicKey

# binary layout for an account key link transaction
struct AccountKeyLinkTransaction
	inline Transaction
	inline AccountKeyLinkTransactionBody

# binary layout for a non-verifiable account key link transaction
struct NonVerifiableAccountKeyLinkTransaction
	inline NonVerifiableTransaction
	inline AccountKeyLinkTransactionBody
