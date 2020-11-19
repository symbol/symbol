/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
