import "types.cats"

# enumeration of account types
enum AccountType : uint8
	# account is not linked to another account
	UNLINKED = 0

	# account is a balance-holding account that is linked to a remote harvester account
	MAIN = 1

	# account is a remote harvester account that is linked to a balance-holding account
	REMOTE = 2

	# account is a remote harvester eligible account that is unlinked
	# \note this allows an account that has previously been used as remote to be reused as a remote
	REMOTE_UNLINKED = 3

# enumeration of account key type flags
@is_bitwise
enum AccountKeyTypeFlags : uint8
	# unset key
	UNSET = 0x00

	# linked account public key
	# \note this can be either a remote or main account public key depending on context
	LINKED = 0x01

	# node public key on which remote is allowed to harvest
	NODE = 0x02

	# VRF public key
	VRF = 0x04

# enumeration of account state formats
enum AccountStateFormat : uint8
	# regular account
	REGULAR = 0

	# high value account eligible to harvest
	HIGH_VALUE = 1

# pinned voting key
struct PinnedVotingKey
	# voting key
	voting_key = VotingPublicKey

	# start finalization epoch
	start_epoch = FinalizationEpoch

	# end finalization epoch
	end_epoch = FinalizationEpoch

# temporal importance information
struct ImportanceSnapshot
	# account importance
	importance = Importance

	# importance height
	height = ImportanceHeight

# account activity bucket
struct HeightActivityBucket
	# activity start height
	start_height = ImportanceHeight

	# total fees paid by account
	total_fees_paid = Amount

	# number of times account has been used as a beneficiary
	beneficiary_count = uint32

	# raw importance score
	raw_score = uint64
