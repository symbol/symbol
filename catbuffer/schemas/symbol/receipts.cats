import "entity.cats"
import "receipt_type.cats"

# Receipts provide proof for every state change not retrievable from the block.
@size(size)
@initializes(type, RECEIPT_TYPE)
@discriminator(type)
@is_aligned
abstract struct Receipt
	inline SizePrefixedEntity

	# Receipt version.
	version = uint16

	# Type of receipt.
	type = ReceiptType

# An invisible state change triggered a mosaic transfer.
inline struct BalanceTransferReceipt
	inline Receipt

	# Transferred mosaic
	mosaic = Mosaic

	# Address of the sender account.
	sender_address = Address

	# Address of the recipient account.
	recipient_address = Address

# An invisible state change modified an account's balance.
inline struct BalanceChangeReceipt
	inline Receipt

	# Modified mosaic.
	mosaic = Mosaic

	# Address of the affected account.
	target_address = Address

# Receipt generated when transaction fees are credited to a block harvester.
struct HarvestFeeReceipt
	RECEIPT_TYPE = make_const(ReceiptType, HARVEST_FEE)

	inline BalanceChangeReceipt

# Network currency mosaics were created due to [inflation](/concepts/inflation).
struct InflationReceipt
	RECEIPT_TYPE = make_const(ReceiptType, INFLATION)

	inline Receipt

	# Created mosaic.
	mosaic = Mosaic
