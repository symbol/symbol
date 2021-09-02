import "transaction.cats"

# binary layout for a vrf key link transaction
struct VrfKeyLinkTransactionBody
	# linked public key
	linked_public_key = PublicKey

	# link action
	link_action = LinkAction

# binary layout for a non-embedded vrf key link transaction
struct VrfKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VRF_KEY_LINK)

	inline Transaction
	inline VrfKeyLinkTransactionBody

# binary layout for an embedded vrf key link transaction
struct EmbeddedVrfKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VRF_KEY_LINK)

	inline EmbeddedTransaction
	inline VrfKeyLinkTransactionBody
