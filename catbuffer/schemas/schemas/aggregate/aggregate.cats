import "transaction.cats"

# a cosignature
struct Cosignature
	# cosigner public key
	signer = Key

	# cosigner signature
	signature = Signature

# a detached cosignature
struct DetachedCosignature
	inline Cosignature

	# hash of the corresponding parent
	parentHash = Hash256

# binary layout for an aggregate transaction
struct AggregateTransactionBody
	# transaction payload size in bytes
	# \note this is the total number of bytes occupied by all sub-transactions
	payloadSize = uint32

	# sub-transaction data (transactions are variable sized and payload size is in bytes)
	transactions = vararray(Transaction, payloadSize)

	# cosignatures data (fills remaining body space after transactions)
	cosignatures = array(Cosignature, __FILL__)

# binary layout for an aggregate complete transaction
struct AggregateCompleteTransaction
	const uint8 version = 2
	const EntityType entityType = 0x4141

	inline Transaction
	inline AggregateTransactionBody

# binary layout for an aggregate bonded transaction
struct AggregateBondedTransaction
	const uint8 version = 2
	const EntityType entityType = 0x4142

	inline Transaction
	inline AggregateTransactionBody
