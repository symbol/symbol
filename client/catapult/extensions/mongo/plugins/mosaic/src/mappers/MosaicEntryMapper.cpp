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

#include "MosaicEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/mosaic/src/state/MosaicEntry.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamProperties(bson_stream::document& builder, const model::MosaicProperties& properties) {
			auto propertiesArray = builder << "properties" << bson_stream::open_array;
			for (const auto& property : properties)
				propertiesArray
						<< bson_stream::open_document
							<< "id" << utils::to_underlying_type(property.Id)
							<< "value" << static_cast<int64_t>(property.Value)
						<< bson_stream::close_document;
			propertiesArray << bson_stream::close_array;
		}

		void StreamMosaicEntryMetadata(bson_stream::document& builder) {
			builder << "meta" << bson_stream::open_document << bson_stream::close_document;
		}
	}

	bsoncxx::document::value ToDbModel(const state::MosaicEntry& entry) {
		const auto& definition = entry.definition();
		bson_stream::document builder;
		StreamMosaicEntryMetadata(builder);
		auto doc = builder
				<< "mosaic" << bson_stream::open_document
					<< "mosaicId" << ToInt64(entry.mosaicId())
					<< "supply" << ToInt64(entry.supply())
					<< "height" << ToInt64(definition.height())
					<< "owner" << ToBinary(definition.owner())
					<< "revision" << static_cast<int32_t>(definition.revision());

		StreamProperties(builder, definition.properties());

		doc << bson_stream::close_document;

		return builder << bson_stream::finalize;
	}

	// endregion
}}}
