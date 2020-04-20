import "transaction.cats"

# binary layout for a vrf key link transaction
struct VrfKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = Key

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded vrf key link transaction
struct VrfKeyLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4243

	inline Transaction
	inline VrfKeyLinkTransactionBody

# binary layout for an embedded vrf key link transaction
struct EmbeddedVrfKeyLinkTransaction
	inline EmbeddedTransaction
	inline VrfKeyLinkTransactionBody
