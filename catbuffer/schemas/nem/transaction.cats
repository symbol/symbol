import "types.cats"
import "transaction_type.cats"

# enumeration of network types
enum NetworkType : uint16
	# main network
	mainnet = 0x68

	# test network
	testnet = 0x98

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
	signerPublicKeySize = make_reserved(uint32, 32)

	# transaction public key
	signerPublicKey = PublicKey

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp
