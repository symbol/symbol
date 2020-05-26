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

#include "MongoNamespaceCacheStorage.h"
#include "MongoNamespaceCacheStorageUtils.h"
#include "src/mappers/NamespaceDescriptorMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/namespace/src/cache/NamespaceCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct NamespaceCacheTraits {
			using CacheType = cache::NamespaceCache;
			using CacheDeltaType = cache::NamespaceCacheDelta;
			using KeyType = NamespaceId;
			using ModelType = NamespaceDescriptor;
			using IdContainerType = std::unordered_set<KeyType, utils::BaseValueHasher<KeyType>>;
			using ElementContainerType = std::unordered_set<const state::RootNamespaceHistory*>;

			static constexpr auto Collection_Name = "namespaces";
			static constexpr auto Id_Property_Name = "namespace.level0";

			static auto GetId(const state::RootNamespaceHistory& history) {
				return history.id();
			}

			static auto MapToMongoId(NamespaceId id) {
				return mappers::ToInt64(id);
			}

			static auto MapToMongoDocument(const ModelType& descriptor) {
				return plugins::ToDbModel(descriptor);
			}

			static auto MapToMongoModels(const state::RootNamespaceHistory& history, model::NetworkIdentifier) {
				return NamespaceDescriptorsFromHistory(history);
			}
		};
	}

	DEFINE_MONGO_HISTORICAL_CACHE_STORAGE(Namespace, NamespaceCacheTraits)
}}}
