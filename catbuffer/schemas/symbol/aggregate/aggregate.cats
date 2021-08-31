import "aggregate/cosignature.cats"
import "transaction.cats"

# binary layout for an aggregate transaction
struct AggregateTransactionBody
	# aggregate hash of an aggregate's transactions
	transactionsHash = Hash256

	# transaction payload size in bytes
	# \note this is the total number of bytes occupied by all sub-transactions
	payloadSize = uint32

	# reserved padding to align end of AggregateTransactionHeader on 8-byte boundary
	aggregateTransactionHeader_Reserved1 = make_reserved(uint32, 0)

	# sub-transaction data (transactions are variable sized and payload size is in bytes)
	transactions = array(EmbeddedTransaction, size=payloadSize)

	# cosignatures data (fills remaining body space after transactions)
	cosignatures = array(Cosignature, __FILL__)

# binary layout for an aggregate complete transaction
struct AggregateCompleteTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, aggregate_complete)

	inline Transaction
	inline AggregateTransactionBody

# binary layout for an aggregate bonded transaction
struct AggregateBondedTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, aggregate_bonded)

	inline Transaction
	inline AggregateTransactionBody
