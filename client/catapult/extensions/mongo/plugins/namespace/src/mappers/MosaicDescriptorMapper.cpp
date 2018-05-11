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

#include "MosaicDescriptorMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/state/MosaicEntry.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamProperties(bson_stream::document& builder, const model::MosaicProperties& properties) {
			auto propertiesArray = builder << "properties" << bson_stream::open_array;
			for (const auto& property : properties)
				propertiesArray << static_cast<int64_t>(property.Value);

			propertiesArray << bson_stream::close_array;
		}

		void StreamDescriptorMetadata(bson_stream::document& builder, const MosaicDescriptor& descriptor) {
			builder << "meta"
					<< bson_stream::open_document
						<< "active" << descriptor.IsActive
						<< "index" << static_cast<int32_t>(descriptor.Index)
					<< bson_stream::close_document;
		}
	}

	bsoncxx::document::value ToDbModel(const MosaicDescriptor& descriptor) {
		const auto& entry = *descriptor.pEntry;
		const auto& definition = entry.definition();
		bson_stream::document builder;
		StreamDescriptorMetadata(builder, descriptor);
		auto doc = builder
				<< "mosaic" << bson_stream::open_document
					<< "namespaceId" << ToInt64(entry.namespaceId())
					<< "mosaicId" << ToInt64(entry.mosaicId())
					<< "supply" << ToInt64(entry.supply())
					<< "height" << ToInt64(definition.height())
					<< "owner" << ToBinary(definition.owner());

		StreamProperties(builder, definition.properties());

		doc
				<< "levy" << bson_stream::open_document // document left blank until levy structure is decided
				<< bson_stream::close_document
				<< bson_stream::close_document;

		return builder << bson_stream::finalize;
	}

	// endregion

	// region ToModel

	namespace {
		using PropertyValuesContainer = model::MosaicProperties::PropertyValuesContainer;

		PropertyValuesContainer ReadProperties(const bsoncxx::array::view& dbProperties) {
			PropertyValuesContainer container{};
			auto i = 0u;
			for (const auto& property : dbProperties)
				container[i++] = static_cast<uint64_t>(property.get_int64().value);

			return container;
		}
	}

	MosaicDescriptor ToMosaicDescriptor(const bsoncxx::document::view& document) {
		// metadata
		auto dbMeta = document["meta"];
		auto isActive = dbMeta["active"].get_bool();
		auto index = static_cast<uint32_t>(dbMeta["index"].get_int32());

		// data
		auto dbMosaic = document["mosaic"];
		auto namespaceId = GetValue64<NamespaceId>(dbMosaic["namespaceId"]);
		auto id = GetValue64<MosaicId>(dbMosaic["mosaicId"]);
		auto supply = GetValue64<Amount>(dbMosaic["supply"]);

		auto height = GetValue64<Height>(dbMosaic["height"]);
		Key owner;
		DbBinaryToModelArray(owner, dbMosaic["owner"].get_binary());
		auto container = ReadProperties(dbMosaic["properties"].get_array().value);

		auto pEntry = std::make_shared<state::MosaicEntry>(
				namespaceId,
				id,
				state::MosaicDefinition(height, owner, model::MosaicProperties::FromValues(container)));
		pEntry->increaseSupply(supply);
		return MosaicDescriptor(pEntry, index, isActive);
	}

	// endregion
}}}
