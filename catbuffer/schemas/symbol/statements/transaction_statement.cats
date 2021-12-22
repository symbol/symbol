import "statements/receipt_source.cats"
import "receipts.cats"

# Collection of receipts scoped to a single source (transaction or block).
struct TransactionStatement
	inline ReceiptSource

	# Number of receipts.
	receipt_count = uint32

	# Receipts.
	receipts = array(Receipt, receipt_count)
