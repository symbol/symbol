import "transaction.cats"

# binary layout for a voting key link transaction
struct VotingKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = VotingKey

	# start finalization epoch
	startEpoch = FinalizationEpoch

	# end finalization epoch
	endEpoch = FinalizationEpoch

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded voting key link transaction
struct VotingKeyLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4143

	inline Transaction
	inline VotingKeyLinkTransactionBody

# binary layout for an embedded voting key link transaction
struct EmbeddedVotingKeyLinkTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4143

	inline EmbeddedTransaction
	inline VotingKeyLinkTransactionBody
