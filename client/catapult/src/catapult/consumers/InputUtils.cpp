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
			for (const auto& txElement : element.Transactions)
				hashes.insert(&txElement.EntityHash);

		return hashes;
	}

	void ExtractEntityInfos(const TransactionElements& elements, model::WeakEntityInfos& entityInfos) {
		for (const auto& element : elements) {
			if (element.Skip)
				continue;

			entityInfos.push_back(model::WeakEntityInfo(element.Transaction, element.EntityHash));
		}
	}

	TransactionInfos CollectRevertedTransactionInfos(
			const utils::HashPointerSet& addedTransactionHashes,
			TransactionInfos&& removedTransactionInfos) {
		TransactionInfos revertedTransactionInfos;
		for (auto& info : removedTransactionInfos) {
			if (addedTransactionHashes.cend() != addedTransactionHashes.find(&info.EntityHash))
				continue;

			revertedTransactionInfos.push_back(std::move(info));
		}

		return revertedTransactionInfos;
	}
}}
