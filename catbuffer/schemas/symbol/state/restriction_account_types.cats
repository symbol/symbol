import "entity.cats"
import "transaction_type.cats"

# binary layout for address based account restriction
struct AccountRestrictionAddressValue
	# number of restrictions for a particular account
	restriction_values_count = uint64

	# restriction values
	restriction_values = array(Address, restriction_values_count)

# binary layout for mosaic id based account restriction
struct AccountRestrictionMosaicValue
	# number of restrictions for a particular account
	restriction_values_count = uint64

	# restriction values
	restriction_values = array(MosaicId, restriction_values_count)

# binary layout for transaction type based account restriction
struct AccountRestrictionTransactionTypeValue
	# number of restrictions for a particular account
	restriction_values_count = uint64

	# restriction values
	restriction_values = array(TransactionType, restriction_values_count)

# binary layout for account restrictions
struct AccountRestrictionsInfo
	# raw restriction flags
	restriction_flags = AccountRestrictionFlags

	#  address restrictions
	address_restrictions = AccountRestrictionAddressValue if ADDRESS in restriction_flags

	# mosaic identifier restrictions
	mosaic_id_restrictions = AccountRestrictionMosaicValue if MOSAIC_ID in restriction_flags

	# transaction type restrictions
	transaction_type_restrictions = AccountRestrictionTransactionTypeValue if TRANSACTION_TYPE in restriction_flags
