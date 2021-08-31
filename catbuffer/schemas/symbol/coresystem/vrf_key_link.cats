import "transaction.cats"

# binary layout for a vrf key link transaction
struct VrfKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = PublicKey

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded vrf key link transaction
struct VrfKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, vrf_key_link)

	inline Transaction
	inline VrfKeyLinkTransactionBody

# binary layout for an embedded vrf key link transaction
struct EmbeddedVrfKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, vrf_key_link)

	inline EmbeddedTransaction
	inline VrfKeyLinkTransactionBody
