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

#include "MemoryUtCacheUtils.h"

namespace catapult { namespace cache {

	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(const MemoryUtCacheView& utCacheView, uint32_t count) {
		std::vector<const model::TransactionInfo*> transactionInfoPointers;
		transactionInfoPointers.reserve(std::min<size_t>(utCacheView.size(), count));

		if (0 != count) {
			utCacheView.forEach([count, &transactionInfoPointers](const auto& transactionInfo) {
				transactionInfoPointers.push_back(&transactionInfo);
				return transactionInfoPointers.size() != count;
			});
		}

		return transactionInfoPointers;
	}

	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t count,
			const predicate<const model::TransactionInfo&>& filter) {
		std::vector<const model::TransactionInfo*> transactionInfoPointers;
		transactionInfoPointers.reserve(std::min<size_t>(utCacheView.size(), count));

		if (0 != count) {
			utCacheView.forEach([count, filter, &transactionInfoPointers](const auto& transactionInfo) {
				if (filter(transactionInfo))
					transactionInfoPointers.push_back(&transactionInfo);

				return transactionInfoPointers.size() != count;
			});
		}

		return transactionInfoPointers;
	}

	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t count,
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
		std::vector<const model::TransactionInfo*> candidateTransactionInfoPointers;
		candidateTransactionInfoPointers.reserve(std::min<size_t>(utCacheView.size(), count));
		for (const auto* pTransactionInfo : allTransactionInfoPointers) {
			if (count == candidateTransactionInfoPointers.size())
				break;

			if (filter(*pTransactionInfo))
				candidateTransactionInfoPointers.push_back(pTransactionInfo);
		}

		return candidateTransactionInfoPointers;
	}
}}
