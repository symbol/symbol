import "transaction.cats"

# Shared content between VrfKeyLinkTransaction and EmbeddedVrfKeyLinkTransaction.
inline struct VrfKeyLinkTransactionBody
	# Linked VRF public key.
	linked_public_key = PublicKey

	# Account link action.
	link_action = LinkAction

# Link an account with a VRF public key required for harvesting.
#
# Announce a VrfKeyLinkTransaction to link an account with a VRF public key.
# The linked key is used to randomize block production and leader/participant selection.
#
# This transaction is required for all accounts wishing to
# [harvest](/concepts/harvesting.html).
struct VrfKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VRF_KEY_LINK)

	inline Transaction
	inline VrfKeyLinkTransactionBody

# Embedded version of VrfKeyLinkTransaction.
struct EmbeddedVrfKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VRF_KEY_LINK)

	inline EmbeddedTransaction
	inline VrfKeyLinkTransactionBody
