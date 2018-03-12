#include "RegisterNamespaceMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/RegisterNamespaceTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			if (0 == transaction.NamespaceNameSize)
				CATAPULT_THROW_RUNTIME_ERROR("cannot map register namespace transaction without name");

			builder << "namespaceType" << utils::to_underlying_type(transaction.NamespaceType);

			if (transaction.IsRootRegistration())
				builder << "duration" << ToInt64(transaction.Duration);
			else
				builder << "parentId" << ToInt64(transaction.ParentId);

			builder
					<< "namespaceId" << ToInt64(transaction.NamespaceId)
					<< "name" << ToBinary(transaction.NamePtr(), transaction.NamespaceNameSize);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateRegisterNamespaceTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::RegisterNamespaceTransaction, model::EmbeddedRegisterNamespaceTransaction>(
				StreamTransaction<model::RegisterNamespaceTransaction>,
				StreamTransaction<model::EmbeddedRegisterNamespaceTransaction>);
	}
}}}
