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
#include "catapult/model/HeightHashPair.h"
#include "catapult/subscribers/StateChangeInfo.h"
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

	/// Steps in a commit operation.
	enum class CommitOperationStep : uint16_t {
		/// Blocks were written to disk.
		Blocks_Written,

		/// State was written to disk.
		State_Written,

		/// Everything was updated.
		All_Updated
	};

	/// Handlers used by the block chain sync consumer.
	struct BlockChainSyncHandlers {
	public:
		/// Prototype for checking block difficulties.
		using DifficultyCheckerFunc = predicate<const std::vector<const model::Block*>&, const cache::CatapultCache&>;

		/// Prototype for retrieving a finalized height hash pair.
		using FinalizedHeightHashPairSupplierFunc = supplier<model::HeightHashPair>;

		/// Prototype for undoing a block.
		/// \note This is called with all rolled back blocks and the (new) common block.
		using UndoBlockFunc = consumer<const model::BlockElement&, observers::ObserverState&, UndoBlockType>;

		/// Prototype for state change notification.
		using StateChangeFunc = consumer<const subscribers::StateChangeInfo&>;

		/// Prototype for pre state written notification.
		using PreStateWrittenFunc = consumer<const cache::CatapultCacheDelta&, Height>;

		/// Prototype for transaction change notification.
		using TransactionsChangeFunc = consumer<const TransactionsChangeInfo&>;

		/// Prototype for commit step notification.
		using CommitStepFunc = consumer<CommitOperationStep>;

	public:
		/// Checks all difficulties in a block chain for correctness.
		DifficultyCheckerFunc DifficultyChecker;

		/// Supplies the local finalized height hash pair.
		FinalizedHeightHashPairSupplierFunc LocalFinalizedHeightHashPairSupplier;

		/// Supplies the network finalized height hash pair.
		FinalizedHeightHashPairSupplierFunc NetworkFinalizedHeightHashPairSupplier;

		/// Processes (validates and executes) a block chain.
		BlockChainProcessor Processor;

		/// Undoes a block and updates a cache.
		UndoBlockFunc UndoBlock;

		/// Called with state change info to indicate a state change.
		StateChangeFunc StateChange;

		/// Called after state change but before state written checkpoint.
		PreStateWrittenFunc PreStateWritten;

		/// Called with the hashes of confirmed transactions and the infos of reverted transactions when transaction statuses change.
		TransactionsChangeFunc TransactionsChange;

		/// Called with the commit operation step.
		CommitStepFunc CommitStep;
	};
}}
