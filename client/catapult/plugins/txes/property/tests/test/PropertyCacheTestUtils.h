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
#include "PropertyTestUtils.h"
#include "src/cache/PropertyCache.h"
#include "src/cache/PropertyCacheStorage.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"

namespace catapult { namespace test {

	/// Cache factory for creating a catapult cache composed of property cache and core caches.
	struct PropertyCacheFactory {
	private:
		static auto CreateSubCachesWithPropertyCache() {
			auto cacheId = cache::PropertyCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			subCaches[cacheId] = MakeSubCachePlugin<cache::PropertyCache, cache::PropertyCacheStorage>(model::NetworkIdentifier::Zero);
			return subCaches;
		}

	public:
		/// Creates an empty catapult cache around default configuration.
		static cache::CatapultCache Create() {
			return Create(model::BlockChainConfiguration::Uninitialized());
		}

		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration& config) {
			auto subCaches = CreateSubCachesWithPropertyCache();
			CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
			return cache::CatapultCache(std::move(subCaches));
		}
	};

	// region PopulateCache

	/// Populates \a delta with \a key and \a values.
	template<typename TPropertyValueTraits, typename TOperationTraits = AllowTraits>
	void PopulateCache(
			cache::CatapultCacheDelta& delta,
			const Key& key,
			const std::vector<typename TPropertyValueTraits::ValueType>& values) {
		auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
		auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
		propertyCacheDelta.insert(state::AccountProperties(address));
		auto& accountProperties = propertyCacheDelta.find(address).get();
		auto& accountProperty = accountProperties.property(TPropertyValueTraits::Property_Type);
		for (const auto& value : values)
			TOperationTraits::Add(accountProperty, state::ToVector(value));
	}

	/// Populates \a cache with \a key and \a values.
	template<typename TPropertyValueTraits, typename TOperationTraits = AllowTraits>
	void PopulateCache(cache::CatapultCache& cache, const Key& key, const std::vector<typename TPropertyValueTraits::ValueType>& values) {
		auto delta = cache.createDelta();
		PopulateCache<TPropertyValueTraits, TOperationTraits>(delta, key, values);
		cache.commit(Height(1));
	}

	// endregion
}}
