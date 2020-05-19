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

#include "MetadataEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/metadata/src/state/MetadataEntry.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	bsoncxx::document::value ToDbModel(const state::MetadataEntry& metadataEntry) {
		const auto& key = metadataEntry.key();
		const auto& value = metadataEntry.value();
		bson_stream::document builder;
		auto doc = builder
				<< "metadataEntry" << bson_stream::open_document
					<< "compositeHash" << ToBinary(key.uniqueKey())
					<< "sourceAddress" << ToBinary(key.sourceAddress())
					<< "targetAddress" << ToBinary(key.targetAddress())
					<< "scopedMetadataKey" << static_cast<int64_t>(key.scopedMetadataKey())
					<< "targetId" << static_cast<int64_t>(key.targetId())
					<< "metadataType" << utils::to_underlying_type(metadataEntry.key().metadataType())
					<< "valueSize" << static_cast<int32_t>(value.size());

		if (!value.empty())
			builder << "value" << ToBinary(value.data(), value.size());

		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion
}}}
