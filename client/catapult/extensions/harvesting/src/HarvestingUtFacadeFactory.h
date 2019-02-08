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

#pragma once
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UtUpdater.h"

namespace catapult { namespace harvesting {

	/// Factory for creating unconfirmed transactions facades.
	class HarvestingUtFacadeFactory {
	public:
		/// Facade around unconfirmed transactions cache and updater.
		class HarvestingUtFacade {
		public:
			/// Creates a facade around \a blockTime, \a catapultCache, \a memoryCacheOptions and \a executionConfig.
			HarvestingUtFacade(
					Timestamp blockTime,
					const cache::CatapultCache& catapultCache,
					const cache::MemoryCacheOptions& memoryCacheOptions,
					const chain::ExecutionConfiguration& executionConfig);

		public:
			/// Gets a read only view of the underlying cache.
			cache::MemoryUtCacheView view() const;

		public:
			/// Attempts to apply \a transactionInfo to the cache.
			bool apply(const model::TransactionInfo& transactionInfo);

		private:
			size_t size() const;

		private:
			cache::MemoryUtCache m_utCache;
			chain::UtUpdater m_utUpdater;
		};

	public:
		/// Creates a factory around \a catapultCache, \a memoryCacheOptions and \a executionConfig.
		HarvestingUtFacadeFactory(
				const cache::CatapultCache& catapultCache,
				const cache::MemoryCacheOptions& memoryCacheOptions,
				const chain::ExecutionConfiguration& executionConfig);

	public:
		/// Creates a facade for applying transactions at a given block time (\a blockTime).
		std::unique_ptr<HarvestingUtFacade> create(Timestamp blockTime) const;

	private:
		const cache::CatapultCache& m_catapultCache;
		cache::MemoryCacheOptions m_memoryCacheOptions;
		chain::ExecutionConfiguration m_executionConfig;
	};
}}
