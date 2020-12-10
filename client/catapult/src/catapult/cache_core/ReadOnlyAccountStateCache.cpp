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

#include "ReadOnlyAccountStateCache.h"
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"

namespace catapult { namespace cache {

	ReadOnlyAccountStateCache::ReadOnlyAccountStateCache(const BasicAccountStateCacheView& cache)
			: AddressBasedCache(cache)
			, KeyBasedCache(cache)
			, m_pCache(&cache)
			, m_pCacheDelta(nullptr)
	{}

	ReadOnlyAccountStateCache::ReadOnlyAccountStateCache(const BasicAccountStateCacheDelta& cache)
			: AddressBasedCache(cache)
			, KeyBasedCache(cache)
			, m_pCache(nullptr)
			, m_pCacheDelta(&cache)
	{}

	model::NetworkIdentifier ReadOnlyAccountStateCache::networkIdentifier() const {
		return m_pCache ? m_pCache->networkIdentifier() : m_pCacheDelta->networkIdentifier();
	}

	uint64_t ReadOnlyAccountStateCache::importanceGrouping() const {
		return m_pCache ? m_pCache->importanceGrouping() : m_pCacheDelta->importanceGrouping();
	}

	Amount ReadOnlyAccountStateCache::minHarvesterBalance() const {
		return m_pCache ? m_pCache->minHarvesterBalance() : m_pCacheDelta->minHarvesterBalance();
	}

	Amount ReadOnlyAccountStateCache::maxHarvesterBalance() const {
		return m_pCache ? m_pCache->maxHarvesterBalance() : m_pCacheDelta->maxHarvesterBalance();
	}

	MosaicId ReadOnlyAccountStateCache::harvestingMosaicId() const {
		return m_pCache ? m_pCache->harvestingMosaicId() : m_pCacheDelta->harvestingMosaicId();
	}

	namespace {
		template<typename TSource>
		HighValueAccountStatistics ComputeHighValueAccountStatistics(const TSource& source) {
			HighValueAccountStatistics statistics;
			statistics.HarvestingEligibleAccountsCount = source.addresses().size();

			statistics.VotingEligibleAccountsCount = 0;
			for (const auto& accountHistoryPair : source.accountHistories()) {
				const auto& balanceHistory = accountHistoryPair.second.balance();
				if (Amount() == balanceHistory.get())
					continue;

				++statistics.VotingEligibleAccountsCount;
				statistics.TotalVotingBalance = statistics.TotalVotingBalance + balanceHistory.get();
			}

			return statistics;
		}
	}

	HighValueAccountStatistics ReadOnlyAccountStateCache::highValueAccountStatistics() const {
		return m_pCache
				? ComputeHighValueAccountStatistics(m_pCache->highValueAccounts())
				: ComputeHighValueAccountStatistics(m_pCacheDelta->highValueAccounts());
	}
}}
