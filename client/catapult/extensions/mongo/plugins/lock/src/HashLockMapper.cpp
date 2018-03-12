#include "HashLockMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/HashLockTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "duration" << ToInt64(transaction.Duration)
					<< "mosaicId" << ToInt64(transaction.Mosaic.MosaicId)
					<< "amount" << ToInt64(transaction.Mosaic.Amount)
					<< "hash" << ToBinary(transaction.Hash);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateHashLockTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::HashLockTransaction, model::EmbeddedHashLockTransaction>(
				StreamTransaction<model::HashLockTransaction>,
				StreamTransaction<model::EmbeddedHashLockTransaction>);
	}
}}}
