import "types.cats"

# enumeration of account types
enum AccountType : uint8
	# account is not linked to another account
	unlinked = 0

	# account is a balance-holding account that is linked to a remote harvester account
	main = 1

	# account is a remote harvester account that is linked to a balance-holding account
	remote = 2

	# account is a remote harvester eligible account that is unlinked
	# \note this allows an account that has previously been used as remote to be reused as a remote
	remoteUnlinked = 3

# enumeration of account key type flags
enum AccountKeyTypeFlags : uint8
	# unset key
	unset = 0x00

	# linked account public key
	# \note this can be either a remote or main account public key depending on context
	linked = 0x01

	# node public key on which remote is allowed to harvest
	node = 0x02

	# VRF public key
	vrf = 0x04

# enumeration of account state formats
enum AccountStateFormat : uint8
	# regular account
	regular = 0

	# high value account eligible to harvest
	highValue = 1

# pinned voting key
struct PinnedVotingKey
	# voting key
	votingKey = VotingKey

	# start finalization epoch
	startEpoch = FinalizationEpoch

	# end finalization epoch
	endEpoch = FinalizationEpoch

# temporal importance information
struct ImportanceSnapshot
	# account importance
	importance = Importance

	# importance height
	height = ImportanceHeight

# account activity bucket
struct HeightActivityBucket
	# activity start height
	startHeight = ImportanceHeight

	# total fees paid by account
	totalFeesPaid = Amount

	# number of times account has been used as a beneficiary
	beneficiaryCount = uint32

	# raw importance score
	rawScore = uint64
