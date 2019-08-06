import "types.cats"

# binary layout for account restrictions
struct AccountRestrictionsInfo
	# restriction type
	restrictionType = AccountRestrictionType

	# amount of restrictions for a particular account
	restrictionValuesCount = uint64

	# restrictions values
	restrictionValues = array(AccountRestrictionType, restrictionValuesCount)
