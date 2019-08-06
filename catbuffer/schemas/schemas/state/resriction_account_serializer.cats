import "restriction_account/restriction_account_types.cats"
import "state/restriction_account_serializer_types.cats"

# binary layout for account restrictions serializer
struct AccountRestrictions
	# address in which restrictions are placed
	address = Address

	# size of restrictions 
	restrictionsSize = uint64
	
	# restrictions to be serialized
	restrictions = AccountRestrictionsInfo
