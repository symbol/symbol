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

#include "MongoPropertyCacheStorage.h"
#include "src/mappers/AccountPropertiesMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/property/src/cache/PropertyCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct PropertyCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::PropertyCacheDescriptor> {
			static constexpr auto Collection_Name = "accountProperties";
			static constexpr auto Id_Property_Name = "accountProperties.address";

			static auto MapToMongoId(const KeyType& key) {
				return mappers::ToBinary(key);
			}

			static auto MapToMongoDocument(const ModelType& accountProperties, model::NetworkIdentifier) {
				return plugins::ToDbModel(accountProperties);
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				cache.insert(ToAccountProperties(document));
			}
		};
	}

	DEFINE_MONGO_FLAT_CACHE_STORAGE(Property, PropertyCacheTraits)
}}}
