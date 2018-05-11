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

#include "MongoMosaicCacheStorage.h"
#include "CacheStorageUtils.h"
#include "src/mappers/MosaicDescriptorMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/namespace/src/cache/MosaicCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MosaicCacheTraits {
			using CacheType = cache::MosaicCache;
			using CacheDeltaType = cache::MosaicCacheDelta;
			using KeyType = MosaicId;
			using ModelType = MosaicDescriptor;
			using IdContainerType = std::unordered_set<KeyType, utils::BaseValueHasher<KeyType>>;
			using ElementContainerType = std::unordered_set<const state::MosaicHistory*>;

			static constexpr const char* Collection_Name = "mosaics";
			static constexpr const char* Id_Property_Name = "mosaic.mosaicId";

			static auto GetId(const state::MosaicHistory& history) {
				return history.id();
			}

			static auto MapToMongoId(MosaicId id) {
				return mappers::ToInt64(id);
			}

			static auto MapToMongoDocument(const ModelType& descriptor) {
				return plugins::ToDbModel(descriptor);
			}

			static auto MapToMongoModels(const state::MosaicHistory& history, model::NetworkIdentifier) {
				return MosaicDescriptorsFromHistory(history);
			}

			static auto LoadSortOrder() {
				return document() << "mosaic.namespaceId" << 1 << "mosaic.mosaicId" << 1 << "meta.index" << 1 << finalize;
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				cache.insert(*ToMosaicDescriptor(document).pEntry);
			}
		};
	}

	DEFINE_MONGO_HISTORICAL_CACHE_STORAGE(Mosaic, MosaicCacheTraits)
}}}
