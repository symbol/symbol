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
	address_height = Height

	# public key of account
	public_key = PublicKey

	# height at which public key has been obtained
	public_key_height = Height

	# type of account
	account_type = AccountType

	# account format
	format = AccountStateFormat

	# mask of supplemental public key flags
	supplemental_public_keys_mask = AccountKeyTypeFlags

	# number of voting public keys
	voting_public_keys_count = uint8

	# linked account public key
	linked_public_key = PublicKey if LINKED in supplemental_public_keys_mask

	# node public key
	node_public_key = PublicKey if NODE in supplemental_public_keys_mask

	# vrf public key
	vrf_public_key = PublicKey if VRF in supplemental_public_keys_mask

	# voting public keys
	voting_public_keys = array(PinnedVotingKey, voting_public_keys_count)

	# current importance snapshot of the account
	importance_snapshots = ImportanceSnapshot if HIGH_VALUE equals format

	# activity buckets of the account
	activity_buckets = HeightActivityBuckets if HIGH_VALUE equals format

	# number of total balances (mosaics)
	balances_count = uint16

	# balances of account
	balances = array(Mosaic, balances_count)
