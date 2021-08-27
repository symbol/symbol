import "restriction_account/restriction_account_types.cats"
import "state/restriction_account_types.cats"
import "state/state_header.cats"

# binary layout for account restrictions
struct AccountRestrictions
	inline StateHeader

	# address on which restrictions are placed
	address = Address

	# number of restrictions
	restrictionsCount = uint64

	# account restrictions
	restrictions = array(AccountRestrictionsInfo, restrictionsCount)
