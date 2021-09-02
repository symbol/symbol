import "entity.cats"

# enumeration of account restriction flags
enum AccountRestrictionFlags : uint16
	# restriction type is an address
	ADDRESS = 0x0001

	# restriction type is a mosaic identifier
	MOSAIC_ID = 0x0002

	# restriction type is a transaction type
	TRANSACTION_TYPE = 0x0004

	# restriction is interpreted as outgoing
	OUTGOING = 0x4000

	# restriction is interpreted as blocking (instead of allowing) operation
	BLOCK = 0x8000
