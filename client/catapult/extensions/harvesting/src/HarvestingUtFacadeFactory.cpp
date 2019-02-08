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

#include "HarvestingUtFacadeFactory.h"

namespace catapult { namespace harvesting {

	// region HarvestingUtFacade

	HarvestingUtFacadeFactory::HarvestingUtFacade::HarvestingUtFacade(
			Timestamp blockTime,
			const cache::CatapultCache& catapultCache,
			const cache::MemoryCacheOptions& memoryCacheOptions,
			const chain::ExecutionConfiguration& executionConfig)
			: m_utCache(memoryCacheOptions)
			, m_utUpdater(
					m_utCache,
					catapultCache,
					BlockFeeMultiplier(0), // no fee multiplier filtering
					executionConfig,
					[blockTime]() { return blockTime; },
					[](const auto&, const auto&, auto) {}, // no failed transaction handling
					[](const auto&, const auto&) { return false; }) // no throttle
	{}

	cache::MemoryUtCacheView HarvestingUtFacadeFactory::HarvestingUtFacade::view() const {
		return m_utCache.view();
	}

	bool HarvestingUtFacadeFactory::HarvestingUtFacade::apply(const model::TransactionInfo& transactionInfo) {
		auto initialSize = size();

		std::vector<model::TransactionInfo> utInfos;
		utInfos.push_back(transactionInfo.copy());
		m_utUpdater.update(utInfos);

		return initialSize != size();
	}

	size_t HarvestingUtFacadeFactory::HarvestingUtFacade::size() const {
		return view().size();
	}

	// endregion

	// region HarvestingUtFacadeFactory

	HarvestingUtFacadeFactory::HarvestingUtFacadeFactory(
			const cache::CatapultCache& catapultCache,
			const cache::MemoryCacheOptions& memoryCacheOptions,
			const chain::ExecutionConfiguration& executionConfig)
			: m_catapultCache(catapultCache)
			, m_memoryCacheOptions(memoryCacheOptions)
			, m_executionConfig(executionConfig)
	{}

	std::unique_ptr<HarvestingUtFacadeFactory::HarvestingUtFacade> HarvestingUtFacadeFactory::create(Timestamp blockTime) const {
		return std::make_unique<HarvestingUtFacade>(blockTime, m_catapultCache, m_memoryCacheOptions, m_executionConfig);
	}

	// endregion
}}
