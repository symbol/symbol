#include "SecretProofMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/SecretProofTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "hashAlgorithm" << utils::to_underlying_type(transaction.HashAlgorithm)
					<< "secret" << ToBinary(transaction.Secret)
					<< "proof" << ToBinary(transaction.ProofPtr(), transaction.ProofSize);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateSecretProofTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::SecretProofTransaction, model::EmbeddedSecretProofTransaction>(
				StreamTransaction<model::SecretProofTransaction>,
				StreamTransaction<model::EmbeddedSecretProofTransaction>);
	}
}}}
