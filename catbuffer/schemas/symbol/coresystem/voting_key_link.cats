import "transaction.cats"

# Shared content between VotingKeyLinkTransaction and EmbeddedVotingKeyLinkTransaction.
inline struct VotingKeyLinkTransactionBody
	# Linked voting public key.
	linked_public_key = VotingPublicKey

	# Starting finalization epoch.
	start_epoch = FinalizationEpoch

	# Ending finalization epoch.
	end_epoch = FinalizationEpoch

	# Account link action.
	link_action = LinkAction

# Link an account with a BLS public key required for finalization voting.
#
# This transaction is required for node operators wanting to vote for
# [finalization](/concepts/block.html#finalization).
#
# Announce a VotingKeyLinkTransaction to associate a voting key with an account during a fixed period.
# An account can be linked to up to **3** different voting keys at the same time.
#
# The recommended production setting is to always have at least **2** linked keys with different
# ``endPoint`` values to ensure a key is registered after the first one expires.
#
# See more details in
# [the manual node setup guide](/guides/network/running-a-symbol-node-manually.html#manual-voting-key-renewal).
struct VotingKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VOTING_KEY_LINK)

	inline Transaction
	inline VotingKeyLinkTransactionBody

# Embedded version of VotingKeyLinkTransaction.
struct EmbeddedVotingKeyLinkTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, VOTING_KEY_LINK)

	inline EmbeddedTransaction
	inline VotingKeyLinkTransactionBody
