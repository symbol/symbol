#include "MosaicDefinitionMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/MosaicDefinitionTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		void StreamProperty(bson_stream::array_context& context, model::MosaicPropertyId id, uint64_t value) {
			context << bson_stream::open_document
					<< "id" << utils::to_underlying_type(id)
					<< "value" << static_cast<int64_t>(value)
				<< bson_stream::close_document;
		}

		void StreamRequiredProperties(bson_stream::array_context& context, const model::MosaicPropertiesHeader& propertiesHeader) {
			StreamProperty(context, model::MosaicPropertyId::Flags, utils::to_underlying_type(propertiesHeader.Flags));
			StreamProperty(context, model::MosaicPropertyId::Divisibility, propertiesHeader.Divisibility);
		}

		void StreamOptionalProperties(bson_stream::array_context& context, const model::MosaicProperty* pProperty, size_t numProperties) {
			for (auto i = 0u; i < numProperties; ++i, ++pProperty)
				StreamProperty(context, pProperty->Id, pProperty->Value);
		}

		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			if (0 == transaction.MosaicNameSize)
				CATAPULT_THROW_RUNTIME_ERROR("cannot map mosaic definition transaction without mosaic name");

			builder
					<< "parentId" << ToInt64(transaction.ParentId)
					<< "mosaicId" << ToInt64(transaction.MosaicId)
					<< "name" << ToBinary(transaction.NamePtr(), transaction.MosaicNameSize);
			auto propertiesArray = builder << "properties" << bson_stream::open_array;
			StreamRequiredProperties(propertiesArray, transaction.PropertiesHeader);
			StreamOptionalProperties(propertiesArray, transaction.PropertiesPtr(), transaction.PropertiesHeader.Count);
			propertiesArray << bson_stream::close_array;
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateMosaicDefinitionTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::MosaicDefinitionTransaction, model::EmbeddedMosaicDefinitionTransaction>(
				StreamTransaction<model::MosaicDefinitionTransaction>,
				StreamTransaction<model::EmbeddedMosaicDefinitionTransaction>);
	}
}}}
