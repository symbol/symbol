import "transaction.cats"

# binary layout for a voting key link transaction
struct VotingKeyLinkTransactionBody
	# linked public key
	linked_public_key = VotingPublicKey

	# start finalization epoch
	start_epoch = FinalizationEpoch

	# end finalization epoch
	end_epoch = FinalizationEpoch

	# link action
	link_action = LinkAction

# binary layout for a non-embedded voting key link transaction
struct VotingKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VOTING_KEY_LINK)

	inline Transaction
	inline VotingKeyLinkTransactionBody

# binary layout for an embedded voting key link transaction
struct EmbeddedVotingKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VOTING_KEY_LINK)

	inline EmbeddedTransaction
	inline VotingKeyLinkTransactionBody
