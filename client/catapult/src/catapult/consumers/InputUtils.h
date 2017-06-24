#pragma once
#include "catapult/disruptor/DisruptorElement.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/WeakEntityInfo.h"
#include "catapult/utils/HashSet.h"
#include <unordered_set>

namespace catapult { namespace consumers {

	using disruptor::ConsumerResult;
	using disruptor::BlockElements;
	using disruptor::TransactionElements;

	/// Extracts all transaction hashes from \a elements.
	utils::HashPointerSet ExtractTransactionHashes(const BlockElements& elements);

	/// Extracts all blocks from \a elements.
	std::vector<const model::Block*> ExtractBlocks(const BlockElements& elements);

	/// Extracts all entity infos from \a elements into \a entityInfos.
	void ExtractEntityInfos(
			const TransactionElements& elements,
			model::WeakEntityInfos& entityInfos);

	/// Container for transactions infos.
	using TransactionInfos = std::vector<model::TransactionInfo>;

	/// Filters \a removedTransactionInfos by removing all transaction infos that have a corresponding
	/// hash in \a addedTransactionHashes.
	TransactionInfos CollectRevertedTransactionInfos(
			const utils::HashPointerSet& addedTransactionHashes,
			TransactionInfos&& removedTransactionInfos);
}}
