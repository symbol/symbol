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
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/state/AccountState.h"

namespace catapult {
	namespace cache {
		class BasicAccountStateCacheDelta;
		class BasicAccountStateCacheView;
	}
}

namespace catapult { namespace cache {

	/// Read-only overlay on top of an account cache.
	class ReadOnlyAccountStateCache
			: public ReadOnlyArtifactCache<BasicAccountStateCacheView, BasicAccountStateCacheDelta, Address, state::AccountState>
			, public ReadOnlyArtifactCache<BasicAccountStateCacheView, BasicAccountStateCacheDelta, Key, state::AccountState> {
	private:
		template<typename TCacheKey, typename TCacheValue>
		using ReadOnlySubCache = ReadOnlyArtifactCache<BasicAccountStateCacheView, BasicAccountStateCacheDelta, TCacheKey, TCacheValue>;

		using AddressBasedCache = ReadOnlySubCache<Address, state::AccountState>;
		using KeyBasedCache = ReadOnlySubCache<Key, state::AccountState>;

	public:
		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyAccountStateCache(const BasicAccountStateCacheView& cache);

		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyAccountStateCache(const BasicAccountStateCacheDelta& cache);

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const;

		/// Gets the network importance grouping.
		uint64_t importanceGrouping() const;

		/// Gets the minimum harvester balance.
		Amount minHarvesterBalance() const;

		/// Gets the maximum harvester balance.
		Amount maxHarvesterBalance() const;

		/// Gets the harvesting mosaic id.
		MosaicId harvestingMosaicId() const;

	public:
		using AddressBasedCache::size;
		using AddressBasedCache::contains;
		using AddressBasedCache::find;

		using KeyBasedCache::contains;
		using KeyBasedCache::find;

	private:
		const BasicAccountStateCacheView* m_pCache;
		const BasicAccountStateCacheDelta* m_pCacheDelta;
	};
}}
