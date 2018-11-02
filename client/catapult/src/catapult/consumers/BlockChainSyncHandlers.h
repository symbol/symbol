/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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

	/// Type of block passed to undo block handler.
	enum class UndoBlockType {
		/// Rolled back block.
		Rollback,
		/// New common block.
		Common
	};

	/// Handlers used by the block chain sync consumer.
	struct BlockChainSyncHandlers {
	public:
		/// Prototype for checking block difficulties.
		using DifficultyCheckerFunc = predicate<const std::vector<const model::Block*>&, const cache::CatapultCache&>;

		/// Prototype for undoing a block.
		/// \note This is called with all rolled back blocks and the (new) common block.
		using UndoBlockFunc = consumer<const model::BlockElement&, const observers::ObserverState&, UndoBlockType>;

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
