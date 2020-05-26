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
			builder
					<< "flags" << utils::to_underlying_type(properties.flags())
					<< "divisibility" << properties.divisibility()
					<< "duration" << ToInt64(properties.duration());
		}
	}

	bsoncxx::document::value ToDbModel(const state::MosaicEntry& mosaicEntry) {
		const auto& definition = mosaicEntry.definition();
		bson_stream::document builder;
		auto doc = builder
				<< "mosaic" << bson_stream::open_document
					<< "id" << ToInt64(mosaicEntry.mosaicId())
					<< "supply" << ToInt64(mosaicEntry.supply())
					<< "startHeight" << ToInt64(definition.startHeight())
					<< "ownerAddress" << ToBinary(definition.ownerAddress())
					<< "revision" << static_cast<int32_t>(definition.revision());

		StreamProperties(builder, definition.properties());

		doc << bson_stream::close_document;

		return builder << bson_stream::finalize;
	}

	// endregion
}}}
