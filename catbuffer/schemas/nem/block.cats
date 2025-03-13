import "transaction.cats"

# enumeration of block types
enum BlockType : uint32
	# nemesis block
	NEMESIS = 0xFFFFFFFF

	# normal block
	NORMAL = 0x000000001

# binary layout for a block
struct Block
	# block type
	type = BlockType

	inline EntityBody

	# previous block hash outer size
	previous_block_hash_outer_size = make_reserved(uint32, 36)

	# previous block hash
	previous_block_hash = inline SizePrefixedHash256

	# block height
	height = Height

	# transactions count
	transactions_count = uint32

	# transactions
	transactions = array(Transaction, transactions_count)
