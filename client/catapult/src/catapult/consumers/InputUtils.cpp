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
			if (element.Skip)
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
