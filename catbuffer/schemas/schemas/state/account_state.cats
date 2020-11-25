import "state/account_state_types.cats"
import "state/state_header.cats"

# account activity buckets
struct HeightActivityBuckets
	# account activity buckets
	buckets = array(HeightActivityBucket, 5)

# binary layout for non-historical account state
struct AccountState
	inline StateHeader

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

	# mask of supplemental public key flags
	supplementalPublicKeysMask = AccountKeyTypeFlags

	# number of voting public keys
	votingPublicKeysCount = uint8

	# linked account public key
	linkedPublicKey = Key if supplementalPublicKeysMask has linked

	# node public key
	nodePublicKey = Key if supplementalPublicKeysMask has node

	# vrf public key
	vrfPublicKey = Key if supplementalPublicKeysMask has vrf

	# voting public keys
	votingPublicKeys = array(PinnedVotingKey, votingPublicKeysCount)

	# current importance snapshot of the account
	importanceSnapshots = ImportanceSnapshot if format equals highValue

	# activity buckets of the account
	activityBuckets = HeightActivityBuckets if format equals highValue

	# number of total balances (mosaics)
	balancesCount = uint16

	# balances of account
	balances = array(Mosaic, balancesCount)
