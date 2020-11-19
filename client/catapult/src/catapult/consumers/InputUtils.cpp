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

#include "InputUtils.h"

namespace catapult { namespace consumers {

	std::vector<const model::Block*> ExtractBlocks(const BlockElements& elements) {
		std::vector<const model::Block*> blocks;
		blocks.reserve(elements.size());

		for (const auto& element : elements)
			blocks.push_back(&element.Block);

		return blocks;
	}

	utils::HashPointerSet ExtractTransactionHashes(const BlockElements& elements) {
		utils::HashPointerSet hashes;

		for (const auto& element : elements)
			for (const auto& transactionElement : element.Transactions)
				hashes.insert(&transactionElement.EntityHash);

		return hashes;
	}

	void ExtractEntityInfos(
			const TransactionElements& elements,
			model::WeakEntityInfos& entityInfos,
			std::vector<size_t>& entityInfoElementIndexes) {
		auto index = 0u;
		for (const auto& element : elements) {
			++index;
			if (disruptor::ConsumerResultSeverity::Success != element.ResultSeverity)
				continue;

			entityInfos.emplace_back(element.Transaction, element.EntityHash);
			entityInfoElementIndexes.push_back(index - 1);
		}
	}

	TransactionInfos CollectRevertedTransactionInfos(
			const utils::HashPointerSet& addedTransactionHashes,
			TransactionInfos&& removedTransactionInfos) {
		TransactionInfos revertedTransactionInfos;
		for (auto& transactionInfo : removedTransactionInfos) {
			if (addedTransactionHashes.cend() != addedTransactionHashes.find(&transactionInfo.EntityHash))
				continue;

			revertedTransactionInfos.push_back(std::move(transactionInfo));
		}

		return revertedTransactionInfos;
	}
}}
