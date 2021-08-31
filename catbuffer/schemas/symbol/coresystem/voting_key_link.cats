import "transaction.cats"

# binary layout for a voting key link transaction
struct VotingKeyLinkTransactionBody
	# linked public key
	linkedPublicKey = VotingPublicKey

	# start finalization epoch
	startEpoch = FinalizationEpoch

	# end finalization epoch
	endEpoch = FinalizationEpoch

	# link action
	linkAction = LinkAction

# binary layout for a non-embedded voting key link transaction
struct VotingKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, voting_key_link)

	inline Transaction
	inline VotingKeyLinkTransactionBody

# binary layout for an embedded voting key link transaction
struct EmbeddedVotingKeyLinkTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, voting_key_link)

	inline EmbeddedTransaction
	inline VotingKeyLinkTransactionBody
