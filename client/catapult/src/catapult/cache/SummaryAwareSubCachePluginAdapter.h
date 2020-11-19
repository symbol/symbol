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

#pragma once
#include "SubCachePluginAdapter.h"

namespace catapult { namespace cache {

	/// CacheStorage implementation for saving and loading summary cache data.
	template<typename TCache>
	class SummaryCacheStorage : public CacheStorage {
	public:
		/// Creates a storage around \a cache.
		explicit SummaryCacheStorage(TCache& cache)
				: m_cache(cache)
				, m_name(std::string(TCache::Name) + "_summary")
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

	protected:
		/// Gets a typed const reference to the underlying cache.
		const TCache& cache() const {
			return m_cache;
		}

		/// Gets a typed reference to the underlying cache.
		TCache& cache() {
			return m_cache;
		}

	private:
		TCache& m_cache;
		std::string m_name;
	};

	/// Specialized sub cache plugin adapter that provides proper handling of summary cache state.
	template<typename TCache, typename TStorageTraits, typename TSummaryCacheStorage>
	class SummaryAwareSubCachePluginAdapter : public SubCachePluginAdapter<TCache, TStorageTraits> {
	private:
		using BaseType = SubCachePluginAdapter<TCache, TStorageTraits>;

	public:
		using BaseType::BaseType;

	public:
		std::unique_ptr<CacheStorage> createStorage() override {
			auto pStorage = BaseType::createStorage();
			return pStorage ? std::move(pStorage) : std::make_unique<TSummaryCacheStorage>(this->cache());
		}
	};
}}
