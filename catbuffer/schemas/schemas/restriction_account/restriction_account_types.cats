import "entity.cats"

# enumeration of account restriction types
enum AccountRestrictionType : uint8
	# restriction type is an address
	address = 0x01

	# restriction type is a mosaic identifier
	mosaicId = 0x02

	# restriction type is a transaction type
	transactionType = 0x04

	# restriction is interpreted as outgoing
	outgoing = 0x40

	# restriction is interpreted as blocking operation
	block = 0x80

# enumeration of account restriction modification actions
enum AccountRestrictionModificationAction : uint8
	# remove account restriction value
	del = 0x00

	# add account restriction value
	add = 0x01

# account restriction basic modification
struct AccountRestrictionModification
	# modification action
	modificationAction = AccountRestrictionModificationAction

# account address restriction modification
struct AccountAddressRestrictionModification
	inline AccountRestrictionModification

	# address restriction value
	value = UnresolvedAddress

# account mosaic restriction modification
struct AccountMosaicRestrictionModification
	inline AccountRestrictionModification

	# mosaic identifier restriction value
	value = UnresolvedMosaicId

# account operation restriction modification
struct AccountOperationRestrictionModification
	inline AccountRestrictionModification

	# transaction type restriction value
	value = EntityType
