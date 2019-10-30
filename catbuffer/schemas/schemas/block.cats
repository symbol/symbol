import "entity.cats"

# binary layout for a block header
struct BlockHeader
	inline SizePrefixedEntity
	inline VerifiableEntity
	inline EntityBody

	# block height
	height = Height

	# number of seconds elapsed since creation of nemesis block
	timestamp = Timestamp

	# block difficulty
	difficulty = Difficulty

	# previous block hash
	previousBlockHash = Hash256

	# hash of the transactions at this block
	transactionsHash = Hash256

	# hash of the receipts at this block
	receiptsHash = Hash256

	# hash of the global chain state at this block
	stateHash = Hash256

	# beneficiary public key designated by harvester
	beneficiaryPublicKey = Key

	# fee multiplier applied to block transactions
	feeMultiplier = uint32
