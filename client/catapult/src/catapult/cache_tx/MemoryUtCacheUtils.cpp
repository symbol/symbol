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

#include "MemoryUtCacheUtils.h"

namespace catapult { namespace cache {

	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t transactionLimit,
			const EmbeddedCountRetriever& countRetriever) {
		return GetFirstTransactionInfoPointers(utCacheView, transactionLimit, countRetriever, [](const auto&) { return true; });
	}

	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t transactionLimit,
			const EmbeddedCountRetriever& countRetriever,
			const predicate<const model::TransactionInfo&>& filter) {
		std::vector<const model::TransactionInfo*> transactionInfoPointers;
		transactionInfoPointers.reserve(std::min<size_t>(utCacheView.size(), transactionLimit));

		if (0 != transactionLimit) {
			uint32_t totalTransactionsCount = 0;
			utCacheView.forEach([transactionLimit, countRetriever, filter, &transactionInfoPointers, &totalTransactionsCount](
					const auto& transactionInfo) {
				auto currentTransactionsCount = countRetriever(*transactionInfo.pEntity);
				if (totalTransactionsCount + currentTransactionsCount > transactionLimit)
					return false;

				if (filter(transactionInfo)) {
					totalTransactionsCount += currentTransactionsCount;
					transactionInfoPointers.push_back(&transactionInfo);
				}

				return true;
			});
		}

		return transactionInfoPointers;
	}

	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t transactionLimit,
			const EmbeddedCountRetriever& countRetriever,
			const predicate<const model::TransactionInfo*, const model::TransactionInfo*>& sortComparer,
			const predicate<const model::TransactionInfo&>& filter) {
		// 1. load all UTs
		std::vector<const model::TransactionInfo*> allTransactionInfoPointers;
		allTransactionInfoPointers.reserve(utCacheView.size());
		utCacheView.forEach([&allTransactionInfoPointers](const auto& transactionInfo) {
			allTransactionInfoPointers.push_back(&transactionInfo);
			return true;
		});

		// 2. sort by predicate (use stable_sort to prefer older when otherwise equal)
		std::sort(allTransactionInfoPointers.begin(), allTransactionInfoPointers.end(), sortComparer);

		// 3. select candidates
		uint32_t totalTransactionsCount = 0;
		std::vector<const model::TransactionInfo*> candidateTransactionInfoPointers;
		candidateTransactionInfoPointers.reserve(std::min<size_t>(utCacheView.size(), transactionLimit));
		for (const auto* pTransactionInfo : allTransactionInfoPointers) {
			auto currentTransactionsCount = countRetriever(*pTransactionInfo->pEntity);
			if (totalTransactionsCount + currentTransactionsCount > transactionLimit)
				break;

			if (filter(*pTransactionInfo)) {
				totalTransactionsCount += currentTransactionsCount;
				candidateTransactionInfoPointers.push_back(pTransactionInfo);
			}
		}

		return candidateTransactionInfoPointers;
	}
}}
