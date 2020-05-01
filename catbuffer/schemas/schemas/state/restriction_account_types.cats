import "entity.cats"

# account restriction flags
enum AccountRestrictionFlags : uint16
	# restriction type is an address
	address = 0x0001

	# restriction type is a mosaic identifier
	mosaicId = 0x0002

	# restriction type is a transaction type
	transactionType = 0x0004

	# restriction is interpreted as outgoing
	outgoing = 0x4000

	# restriction is interpreted as blocking (instead of allowing) operation
	block = 0x8000

# binary layout for address based account restriction
struct AccountRestrictionAddressValue
	# number of restrictions for a particular account
	restrictionValuesCount = uint64

	# restriction values
	restrictionValues = array(Address, restrictionValuesCount)

# binary layout for mosaic id based account restriction
struct AccountRestrictionMosaicValue
	# number of restrictions for a particular account
	restrictionValuesCount = uint64

	# restriction values
	restrictionValues = array(MosaicId, restrictionValuesCount)

# binary layout for transaction type based account restriction
struct AccountRestrictionTransactionTypeValue
	# number of restrictions for a particular account
	restrictionValuesCount = uint64

	# restriction values
	restrictionValues = array(EntityType, restrictionValuesCount)

# binary layout for account restrictions
struct AccountRestrictionsInfo
	# raw restriction flags
	restrictionFlags = AccountRestrictionFlags

	#  address restrictions
	addressRestrictions = AccountRestrictionAddressValue if restrictionFlags has address

	# mosaic identifier restrictions
	mosaicIdRestrictions = AccountRestrictionMosaicValue if restrictionFlags has mosaicId

	# transaction type restrictions
	transactionTypeRestrictions = AccountRestrictionTransactionTypeValue if restrictionFlags has transactionType
