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
	publicKey = PublicKey

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
	linkedPublicKey = PublicKey if linked in supplementalPublicKeysMask

	# node public key
	nodePublicKey = PublicKey if node in supplementalPublicKeysMask

	# vrf public key
	vrfPublicKey = PublicKey if vrf in supplementalPublicKeysMask

	# voting public keys
	votingPublicKeys = array(PinnedVotingKey, votingPublicKeysCount)

	# current importance snapshot of the account
	importanceSnapshots = ImportanceSnapshot if highValue equals format

	# activity buckets of the account
	activityBuckets = HeightActivityBuckets if highValue equals format

	# number of total balances (mosaics)
	balancesCount = uint16

	# balances of account
	balances = array(Mosaic, balancesCount)
