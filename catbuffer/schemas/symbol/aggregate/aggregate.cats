import "aggregate/cosignature.cats"
import "transaction.cats"

# Shared content between AggregateCompleteTransaction and AggregateBondedTransaction.
inline struct AggregateTransactionBody
	# Hash of the aggregate's transaction.
	transactions_hash = Hash256

	# Transaction payload size in bytes.
	#
	# This is the total number of bytes occupied by all embedded transactions,
	# including any padding present.
	payload_size = uint32

	# Reserved padding to align end of AggregateTransactionHeader to an 8-byte boundary.
	aggregate_transaction_header_reserved_1 = make_reserved(uint32, 0)

	# Embedded transaction data.
	#
	# Transactions are variable-sized and the total payload size is in bytes.
	#
	# Embedded transactions cannot be aggregates.
	@is_byte_constrained
	@alignment(8)
	transactions = array(EmbeddedTransaction, payload_size)

	# Cosignatures data.
	#
	# Fills up remaining body space after transactions.
	cosignatures = array(Cosignature, __FILL__)

# Send transactions in batches to different accounts (V1, deprecated).
#
# Use this transaction when all required signatures are available when the transaction is created.
struct AggregateCompleteTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, AGGREGATE_COMPLETE)

	inline Transaction
	inline AggregateTransactionBody

# Send transactions in batches to different accounts (V2, latest).
#
# Use this transaction when all required signatures are available when the transaction is created.
struct AggregateCompleteTransactionV2
	TRANSACTION_VERSION = make_const(uint8, 2)
	TRANSACTION_TYPE = make_const(TransactionType, AGGREGATE_COMPLETE)

	inline Transaction
	inline AggregateTransactionBody

# Propose an arrangement of transactions between different accounts (V1, deprecated).
#
# Use this transaction when not all required signatures are available when the transaction is created.
#
# Missing signatures must be provided using a Cosignature or DetachedCosignature.
#
# To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be
# successfully announced and confirmed.
struct AggregateBondedTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, AGGREGATE_BONDED)

	inline Transaction
	inline AggregateTransactionBody

# Propose an arrangement of transactions between different accounts (V2, latest).
#
# Use this transaction when not all required signatures are available when the transaction is created.
#
# Missing signatures must be provided using a Cosignature or DetachedCosignature.
#
# To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be
# successfully announced and confirmed.
struct AggregateBondedTransactionV2
	TRANSACTION_VERSION = make_const(uint8, 2)
	TRANSACTION_TYPE = make_const(TransactionType, AGGREGATE_BONDED)

	inline Transaction
	inline AggregateTransactionBody
