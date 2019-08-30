import "state/restriction_account_types.cats"

# binary layout for account restrictions
struct AccountRestrictions
	# address on which restrictions are placed
	address = Address

	# number of restrictions
	restrictionsCount = uint64

	# account restrictions
	restrictions = array(AccountRestrictionsInfo, restrictionsCount)
