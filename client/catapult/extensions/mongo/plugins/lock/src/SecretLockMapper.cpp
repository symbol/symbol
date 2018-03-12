#include "SecretLockMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/SecretLockTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "duration" << ToInt64(transaction.Duration)
					<< "mosaicId" << ToInt64(transaction.Mosaic.MosaicId)
					<< "amount" << ToInt64(transaction.Mosaic.Amount)
					<< "hashAlgorithm" << utils::to_underlying_type(transaction.HashAlgorithm)
					<< "secret" << ToBinary(transaction.Secret)
					<< "recipient" << ToBinary(transaction.Recipient);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateSecretLockTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::SecretLockTransaction, model::EmbeddedSecretLockTransaction>(
				StreamTransaction<model::SecretLockTransaction>,
				StreamTransaction<model::EmbeddedSecretLockTransaction>);
	}
}}}
