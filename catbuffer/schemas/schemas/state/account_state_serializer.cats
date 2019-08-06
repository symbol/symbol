import "state/account_state_types.cats"

# binary layout for non-historical account state serializer
struct AccountState
	# address state
	address = Address

	# address state at block height
	addressHeight = Height

	# public key state
	publicKey = Key

	# public key state at block height
	publicKeyHeight = Height

	# link status
	type = AccountType

	# linked key
	linkedKey = Key

	# number of total balances (mosaics) owned
	balanceSize = uint64

	# state mosaic balances
	balances = array(AccountBalance, balanceSize)
