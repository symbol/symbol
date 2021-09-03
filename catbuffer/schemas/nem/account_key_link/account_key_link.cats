import "transaction.cats"

# enumeration of link actions
enum LinkAction : uint32
	# unlink account
	LINK = 0x01

	# link account
	UNLINK = 0x02

# binary layout for an account key link transaction
struct AccountKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_KEY_LINK)

	inline Transaction

	# link action
	link_action = LinkAction

	# [__value__] public key of remote account to which importance should be transferred
	#
	# [size] remote account public key size
	remote_public_key = inline SizePrefixedPublicKey
