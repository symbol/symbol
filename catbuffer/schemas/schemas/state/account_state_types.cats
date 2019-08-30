import "types.cats"

# linked account type that indicates the account's link status
enum AccountType : uint8
	# account is not linked to another account
	unlinked = 0

	# account is a balance-holding account that is linked to a remote harvester account
	main = 1

	# account is a remote harvester account that is linked to a balance-holding account
	remote = 2

	# account is a remote harvester eligible account that is unlinked
	remoteUnlinked = 3

# account state format
enum AccountStateFormat : uint8
	# regular account
	regular = 0

	# high value account eligible to harvest
	highValue = 1

# temporal importance information
struct ImportanceSnapshot
	# account importance
	importance = Importance

	# importance height
	height = ImportanceHeight

# account activity bucket
struct HeightActivityBucket
	# activity start height
	startHeight = ImportanceHeight

	# total fees paid by account
	totalFeesPaid = Amount

	# number of times account has been used as a beneficiary
	beneficiaryCount = uint32

	# raw importance score
	rawScore = uint64
