import "transaction.cats"

# binary layout for a voting key link transaction
struct VotingKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = VotingKey

	# start finalization point
	startPoint = FinalizationPoint

	# end finalization point
	endPoint = FinalizationPoint

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
	inline EmbeddedTransaction
	inline VotingKeyLinkTransactionBody
