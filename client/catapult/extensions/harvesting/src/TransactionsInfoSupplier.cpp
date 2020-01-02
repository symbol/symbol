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

#include "TransactionsInfoSupplier.h"
#include "HarvestingUtFacadeFactory.h"
#include "TransactionFeeMaximizer.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/cache_tx/MemoryUtCacheUtils.h"
#include "catapult/model/FeeUtils.h"

namespace catapult { namespace harvesting {

	namespace {
		using TransactionInfoPointers = std::vector<const model::TransactionInfo*>;

		enum class SortDirection { Ascending, Descending };

		template<SortDirection Direction>
		struct MaxFeeMultiplierComparer {
			bool operator()(const model::TransactionInfo* pLhs, const model::TransactionInfo* pRhs) const {
				auto lhsMaxFeeMultiplier = model::CalculateTransactionMaxFeeMultiplier(*pLhs->pEntity);
				auto rhsMaxFeeMultiplier = model::CalculateTransactionMaxFeeMultiplier(*pRhs->pEntity);
				return SortDirection::Ascending == Direction
						? lhsMaxFeeMultiplier < rhsMaxFeeMultiplier
						: lhsMaxFeeMultiplier > rhsMaxFeeMultiplier;
			}
		};

		TransactionsInfo ToTransactionsInfo(const TransactionInfoPointers& transactionInfoPointers, BlockFeeMultiplier feeMultiplier) {
			TransactionsInfo transactionsInfo;
			transactionsInfo.FeeMultiplier = feeMultiplier;
			transactionsInfo.Transactions.reserve(transactionInfoPointers.size());
			transactionsInfo.TransactionHashes.reserve(transactionInfoPointers.size());

			for (const auto* pTransactionInfo : transactionInfoPointers) {
				transactionsInfo.Transactions.push_back(pTransactionInfo->pEntity);
				transactionsInfo.TransactionHashes.push_back(pTransactionInfo->EntityHash);
			}

			CalculateBlockTransactionsHash(transactionInfoPointers, transactionsInfo.TransactionsHash);
			return transactionsInfo;
		}

		TransactionsInfo SupplyOldest(const cache::MemoryUtCacheView& utCacheView, HarvestingUtFacade& utFacade, uint32_t count) {
			// 1. get first transactions from the ut cache
			auto candidates = cache::GetFirstTransactionInfoPointers(utCacheView, count, [&utFacade](const auto& transactionInfo) {
				return utFacade.apply(transactionInfo);
			});

			// 2. pick the smallest multiplier so that all transactions pass validation
			auto minFeeMultiplier = BlockFeeMultiplier();
			if (!candidates.empty()) {
				auto comparer = MaxFeeMultiplierComparer<SortDirection::Ascending>();
				auto minIter = std::min_element(candidates.cbegin(), candidates.cend(), comparer);
				minFeeMultiplier = model::CalculateTransactionMaxFeeMultiplier(*(*minIter)->pEntity);
			}

			return ToTransactionsInfo(candidates, minFeeMultiplier);
		}

		TransactionsInfo SupplyMinimumFee(const cache::MemoryUtCacheView& utCacheView, HarvestingUtFacade& utFacade, uint32_t count) {
			// 1. get all transactions from the ut cache
			auto comparer = MaxFeeMultiplierComparer<SortDirection::Ascending>();
			auto candidates = cache::GetFirstTransactionInfoPointers(utCacheView, count, comparer, [&utFacade](
					const auto& transactionInfo) {
				return utFacade.apply(transactionInfo);
			});

			// 2. pick the smallest multiplier so that all transactions pass validation
			auto minFeeMultiplier = BlockFeeMultiplier();
			if (!candidates.empty())
				minFeeMultiplier = model::CalculateTransactionMaxFeeMultiplier(*candidates[0]->pEntity);

			return ToTransactionsInfo(candidates, minFeeMultiplier);
		}

		TransactionsInfo SupplyMaximumFee(const cache::MemoryUtCacheView& utCacheView, HarvestingUtFacade& utFacade, uint32_t count) {
			// 1. get all transactions from the ut cache
			auto comparer = MaxFeeMultiplierComparer<SortDirection::Descending>();
			auto maximizer = TransactionFeeMaximizer();
			auto candidates = cache::GetFirstTransactionInfoPointers(utCacheView, count, comparer, [&utFacade, &maximizer](
					const auto& transactionInfo) {
				if (!utFacade.apply(transactionInfo))
					return false;

				maximizer.apply(transactionInfo);
				return true;
			});

			// 2. pick the best fee policy and truncate the transactions and facade
			const auto& bestFeePolicy = maximizer.best();
			candidates.resize(bestFeePolicy.NumTransactions);
			while (utFacade.size() > bestFeePolicy.NumTransactions)
				utFacade.unapply();

			return ToTransactionsInfo(candidates, bestFeePolicy.FeeMultiplier);
		}
	}

	TransactionsInfoSupplier CreateTransactionsInfoSupplier(
			model::TransactionSelectionStrategy strategy,
			const cache::ReadWriteUtCache& utCache) {
		return [strategy, &utCache](auto& utFacade, auto count) {
			auto utCacheView = utCache.view();

			switch (strategy) {
			case model::TransactionSelectionStrategy::Minimize_Fee:
				return SupplyMinimumFee(utCacheView, utFacade, count);

			case model::TransactionSelectionStrategy::Maximize_Fee:
				return SupplyMaximumFee(utCacheView, utFacade, count);

			default:
				return SupplyOldest(utCacheView, utFacade, count);
			}
		};
	}
}}
