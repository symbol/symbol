#pragma once
#include "BlockChainProcessor.h"
#include "StateChangeInfo.h"
#include "catapult/utils/ArraySet.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace chain { struct ObserverState; }
}

namespace catapult { namespace consumers {

	/// Information passed to a transactions change handler.
	struct TransactionsChangeInfo {
	public:
		/// Creates a new transactions change info around \a addedTransactionHashes and \a revertedTransactionInfos.
		TransactionsChangeInfo(
				const utils::HashPointerSet& addedTransactionHashes,
				const std::vector<model::TransactionInfo>& revertedTransactionInfos)
				: AddedTransactionHashes(addedTransactionHashes)
				, RevertedTransactionInfos(revertedTransactionInfos)
		{}

	public:
		/// Hashes of the transactions that were added (newly confirmed).
		const utils::HashPointerSet& AddedTransactionHashes;

		/// Infos of the transactions that were reverted (previously confirmed).
		const std::vector<model::TransactionInfo>& RevertedTransactionInfos;
	};

	/// Handlers used by the block chain sync consumer.
	struct BlockChainSyncHandlers {
	public:
		/// Prototype for checking block difficulties.
		using DifficultyCheckerFunc = predicate<const std::vector<const model::Block*>&, const cache::CatapultCache&>;

		/// Prototype for undoing a block.
		using UndoBlockFunc = consumer<const model::BlockElement&, const observers::ObserverState&>;

		/// Prototype for state change notification.
		using StateChangeFunc = consumer<const StateChangeInfo&>;

		/// Prototype for transaction change notification.
		using TransactionsChangeFunc = consumer<const TransactionsChangeInfo&>;

	public:
		/// Checks all difficulties in a block chain for correctness.
		DifficultyCheckerFunc DifficultyChecker;

		/// Processes (validates and executes) a block chain.
		BlockChainProcessor Processor;

		/// Undoes a block and updates a cache.
		UndoBlockFunc UndoBlock;

		/// Called with state change info to indicate a state change.
		StateChangeFunc StateChange;

		/// Called with the hashes of confirmed transactions and the infos of reverted transactions when transaction statuses change.
		TransactionsChangeFunc TransactionsChange;
	};
}}
