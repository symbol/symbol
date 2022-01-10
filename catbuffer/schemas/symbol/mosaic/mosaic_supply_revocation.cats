import "mosaic/mosaic_types.cats"
import "transaction.cats"

# Shared content between MosaicSupplyRevocationTransaction and EmbeddedMosaicSupplyRevocationTransaction.
inline struct MosaicSupplyRevocationTransactionBody
	# Address from which tokens should be revoked.
	source_address = UnresolvedAddress

	# Revoked mosaic and amount.
	mosaic = UnresolvedMosaic

# Revoke mosaic.
struct MosaicSupplyRevocationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_REVOCATION)

	inline Transaction
	inline MosaicSupplyRevocationTransactionBody

# Embedded version of MosaicSupplyRevocationTransaction.
struct EmbeddedMosaicSupplyRevocationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_REVOCATION)

	inline EmbeddedTransaction
	inline MosaicSupplyRevocationTransactionBody
