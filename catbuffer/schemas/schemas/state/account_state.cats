import "state/account_state_types.cats"

# account activity buckets
struct HeightActivityBuckets
	# account activity buckets
	buckets = array(HeightActivityBucket, 5)

# binary layout for non-historical account state
struct AccountState
	# address of account
	address = Address

	# height at which address has been obtained
	addressHeight = Height

	# public key of account
	publicKey = Key

	# height at which public key has been obtained
	publicKeyHeight = Height

	# type of account
	accountType = AccountType

	# account format
	format = AccountStateFormat

	# mask of supplemental account key flags
	supplementalAccountKeysMask = AccountKeyFlags

	# linked account public key
	linkedPublicKey = Key if supplementalAccountKeysMask has linked

	# vrf public key
	vrfPublicKey = Key if supplementalAccountKeysMask has vrf

	# voting public key
	votingPublicKey = VotingKey if supplementalAccountKeysMask has voting

	# node public key
	nodePublicKey = Key if supplementalAccountKeysMask has node

	# current importance snapshot of the account
	importanceSnapshots = ImportanceSnapshot if format equals highValue

	# activity buckets of the account
	activityBuckets = HeightActivityBuckets if format equals highValue

	# number of total balances (mosaics)
	balancesCount = uint16

	# balances of account
	balances = array(Mosaic, balancesCount)
