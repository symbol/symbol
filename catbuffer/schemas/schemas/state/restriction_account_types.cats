import "entity.cats"

# account restriction flags
enum ExpandedAccountRestrictionFlags : uint16
	# incoming addresses are restricted (allowed)
	address = 0x0001

	# outgoing addresses are restricted (allowed)
	addressOutgoing = 0x4001

	# incoming mosaics are restricted (allowed)
	mosaicId = 0x0002

	# incoming transaction types are restricted (allowed)
	transactionTypeOutgoing = 0x4004

	# incoming addresses are restricted (blocked)
	addressBlock = 0x8001

	# outgoing addresses are restricted (blocked)
	addressOutgoingBlock = 0xC001

	# incoming mosaics are restricted (blocked)
	mosaicIdBlock = 0x8002

	# outgoing transaction types are restricted (blocked)
	transactionTypeOutgoingBlock = 0xC004

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
	restrictionFlags = ExpandedAccountRestrictionFlags

	# incoming allow address restrictions
	restrictions1 = AccountRestrictionAddressValue if restrictionFlags equals address

	# outgoing allow address restrictions
	restrictions2 = AccountRestrictionAddressValue if restrictionFlags equals addressOutgoing

	# incoming block address restrictions
	restrictions3 = AccountRestrictionAddressValue if restrictionFlags equals addressBlock

	# outgoing block address restrictions
	restrictions4 = AccountRestrictionAddressValue if restrictionFlags equals addressOutgoingBlock

	# incoming allow mosaic restrictions
	restrictions5 = AccountRestrictionMosaicValue if restrictionFlags equals mosaicId

	# incoming block mosaic restrictions
	restrictions6 = AccountRestrictionMosaicValue if restrictionFlags equals mosaicIdBlock

	# outgoing allow transaction type restrictions
	restrictions7 = AccountRestrictionTransactionTypeValue if restrictionFlags equals transactionTypeOutgoing

	# outgoing block transaction type restrictions
	restrictions8 = AccountRestrictionTransactionTypeValue if restrictionFlags equals transactionTypeOutgoingBlock
