import "statements/address_resolution_statement.cats"
import "statements/mosaic_resolution_statement.cats"
import "statements/transaction_statement.cats"

# Collection of statements scoped to a block.
struct BlockStatement
	# Number of transaction statements.
	transaction_statement_count = uint32

	# Transaction statements.
	transaction_statements = array(TransactionStatement, transaction_statement_count)

	# Number of address resolution statements.
	address_resolution_statement_count = uint32

	# Address resolution statements.
	address_resolution_statements = array(AddressResolutionStatement, address_resolution_statement_count)

	# Number of mosaic resolution statements.
	mosaic_resolution_statement_count = uint32

	# Mosaic resolution statements.
	mosaic_resolution_statements = array(MosaicResolutionStatement, mosaic_resolution_statement_count)
