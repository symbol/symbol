#pragma once
#include "catapult/disruptor/DisruptorElement.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/WeakEntityInfo.h"
#include "catapult/utils/ArraySet.h"
#include <unordered_set>

namespace catapult { namespace consumers {

	using disruptor::ConsumerResult;
	using disruptor::BlockElements;
	using disruptor::TransactionElements;

	/// Extracts all transaction hashes from \a elements.
	utils::HashPointerSet ExtractTransactionHashes(const BlockElements& elements);

	/// Extracts all blocks from \a elements.
	std::vector<const model::Block*> ExtractBlocks(const BlockElements& elements);

	/// Extracts all non-skipped entity infos from \a elements into \a entityInfos and stores corresponding element indexes
	/// in \a entityInfoElementIndexes.
	void ExtractEntityInfos(
			const TransactionElements& elements,
			model::WeakEntityInfos& entityInfos,
			std::vector<size_t>& entityInfoElementIndexes);

	/// Container for transactions infos.
	using TransactionInfos = std::vector<model::TransactionInfo>;

	/// Filters \a removedTransactionInfos by removing all transaction infos that have a corresponding
	/// hash in \a addedTransactionHashes.
	TransactionInfos CollectRevertedTransactionInfos(
			const utils::HashPointerSet& addedTransactionHashes,
			TransactionInfos&& removedTransactionInfos);
}}
