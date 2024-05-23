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

#include "MongoMosaicRestrictionCacheStorage.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/restriction_mosaic/src/cache/MosaicRestrictionCache.h"
#include "src/mappers/MosaicRestrictionEntryMapper.h"

using namespace bsoncxx::builder::stream;

namespace catapult {
namespace mongo {
	namespace plugins {

		namespace {
			struct MosaicRestrictionCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::MosaicRestrictionCacheDescriptor> {
				static constexpr auto Collection_Name = "mosaicRestrictions";
				static constexpr auto Id_Property_Name = "mosaicRestrictionEntry.compositeHash";

				static auto MapToMongoId(const KeyType& key) {
					return mappers::ToBinary(key);
				}

				static auto MapToMongoDocument(const ModelType& restrictionEntry, model::NetworkIdentifier) {
					return plugins::ToDbModel(restrictionEntry);
				}
			};
		}

		DEFINE_MONGO_FLAT_CACHE_STORAGE(MosaicRestriction, MosaicRestrictionCacheTraits)
	}
}
}
