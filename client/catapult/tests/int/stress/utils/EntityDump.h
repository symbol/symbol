#pragma once

namespace catapult {
	namespace model {
		struct Block;
		struct Transaction;
	}
}

namespace catapult { namespace test {

	/// Outputs the contents of \a tx.
	void EntityDump(const model::Transaction& tx);

	/// Outputs the contents of \a block.
	void EntityDump(const model::Block& block);
}}
