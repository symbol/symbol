#include "MosaicSupplyChangeMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/MosaicSupplyChangeTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "mosaicId" << ToInt64(transaction.MosaicId)
					<< "direction" << utils::to_underlying_type(transaction.Direction)
					<< "delta" << ToInt64(transaction.Delta);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateMosaicSupplyChangeTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::MosaicSupplyChangeTransaction, model::EmbeddedMosaicSupplyChangeTransaction>(
				StreamTransaction<model::MosaicSupplyChangeTransaction>,
				StreamTransaction<model::EmbeddedMosaicSupplyChangeTransaction>);
	}
}}}
