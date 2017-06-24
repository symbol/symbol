#pragma once
#include "BlockChainProcessor.h"
#include "StateChangeInfo.h"
#include "catapult/utils/HashSet.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace chain { struct ObserverState; }
}

namespace catapult { namespace consumers {

	/// Handlers used by the block chain sync consumer.
	struct BlockChainSyncHandlers {
	public:
		/// Prototype for checking block difficulties.
		using DifficultyCheckerFunc = std::function<bool (
				const std::vector<const model::Block*>&,
				const cache::CatapultCache&)>;

		/// Prototype for undoing a block.
		using UndoBlockFunc = std::function<void (const model::BlockElement&, const observers::ObserverState&)>;

		/// Prototype for state change notification.
		using StateChangeFunc = std::function<void (const StateChangeInfo&)>;

		/// Prototype for transaction change notification.
		using TransactionsChangeFunc = std::function<void (
				const utils::HashPointerSet&,
				std::vector<model::TransactionInfo>&&)>;

	public:
		/// Checks all difficulties in a block chain for correctness.
		DifficultyCheckerFunc DifficultyChecker;

		/// Processes (validates and executes) a block chain.
		BlockChainProcessor Processor;

		/// Undoes a block and updates a cache.
		UndoBlockFunc UndoBlock;

		/// Called with state change info to indicate a state change.
		StateChangeFunc StateChange;

		/// Called with the hashes of confirmed transactions and the infos of reverted transactions when
		/// transaction statuses change.
		TransactionsChangeFunc TransactionsChange;
	};
}}
