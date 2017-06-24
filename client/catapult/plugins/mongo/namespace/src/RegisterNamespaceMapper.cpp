#include "RegisterNamespaceMapper.h"
#include "plugins/mongo/coremongo/src/MongoTransactionPluginFactory.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/RegisterNamespaceTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			if (0 == transaction.NamespaceNameSize)
				CATAPULT_THROW_RUNTIME_ERROR("cannot map register namespace transaction without name");

			auto parentId = transaction.ParentId;
			if (transaction.IsRootRegistration()) {
				parentId = Namespace_Base_Id;
				builder << "duration" << ToInt64(transaction.Duration);
			}

			builder
					<< "namespaceType" << utils::to_underlying_type(transaction.NamespaceType)
					<< "parentId" << ToInt64(parentId)
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
