import "types.cats"
import "transaction_type.cats"

# enumeration of network types
enum NetworkType : uint16
	# main network
	MAINNET = 0x68

	# test network
	TESTNET = 0x98

# binary layout for a transaction
struct Transaction
	# transaction type
	type = TransactionType

	# transaction version
	version = uint16

	# transaction network
	network = NetworkType

	# transaction timestamp
	timestamp = Timestamp

	# transaction public key size
	signer_public_key_size = make_reserved(uint32, 32)

	# transaction public key
	signer_public_key = PublicKey

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp
