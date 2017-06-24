#include "MosaicDescriptorMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/state/MosaicEntry.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		void StreamProperties(bson_stream::document& builder, const model::MosaicProperties& properties) {
			auto propertiesArray = builder << "properties" << bson_stream::open_array;
			for (auto iter = properties.cbegin(); properties.cend() != iter; ++iter)
				propertiesArray << static_cast<int64_t>(iter->Value);

			propertiesArray << bson_stream::close_array;
		}

		void StreamDescriptorMetadata(bson_stream::document& builder, const state::MosaicDescriptor& descriptor) {
			builder << "meta"
					<< bson_stream::open_document
						<< "active" << descriptor.IsActive
						<< "index" << static_cast<int32_t>(descriptor.Index)
					<< bson_stream::close_document;
		}
	}

	bsoncxx::document::value ToDbModel(const state::MosaicDescriptor& descriptor) {
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
			PropertyValuesContainer container;
			auto i = 0u;
			for (const auto& property : dbProperties)
				container[i++] = static_cast<uint64_t>(property.get_int64().value);

			return container;
		}
	}

	state::MosaicDescriptor ToMosaicDescriptor(const bsoncxx::document::view& document) {
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
		return state::MosaicDescriptor(pEntry, index, isActive);
	}

	// endregion
}}}
