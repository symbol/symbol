import "aggregate/cosignature.cats"
import "transaction.cats"

# binary layout for an aggregate transaction
struct AggregateTransactionBody
	# aggregate hash of an aggregate's transactions
	transactions_hash = Hash256

	# transaction payload size in bytes
	# \note this is the total number of bytes occupied by all sub-transactions
	payload_size = uint32

	# reserved padding to align end of AggregateTransactionHeader on 8-byte boundary
	aggregate_transaction_header_reserved_1 = make_reserved(uint32, 0)

	# sub-transaction data (transactions are variable sized and payload size is in bytes)
	transactions = array(EmbeddedTransaction, size=payload_size)

	# cosignatures data (fills remaining body space after transactions)
	cosignatures = array(Cosignature, __FILL__)

# binary layout for an aggregate complete transaction
struct AggregateCompleteTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, AGGREGATE_COMPLETE)

	inline Transaction
	inline AggregateTransactionBody

# binary layout for an aggregate bonded transaction
struct AggregateBondedTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, AGGREGATE_BONDED)

	inline Transaction
	inline AggregateTransactionBody
