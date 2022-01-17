import "entity.cats"

# Enumeration of account restriction flags.
@is_bitwise
enum AccountRestrictionFlags : uint16
	# Restriction type is an address.
	ADDRESS = 0x0001

	# Restriction type is a mosaic identifier.
	MOSAIC_ID = 0x0002

	# Restriction type is a transaction type.
	TRANSACTION_TYPE = 0x0004

	# Restriction is interpreted as outgoing.
	OUTGOING = 0x4000

	# Restriction is interpreted as blocking (instead of allowing) operation.
	BLOCK = 0x8000
