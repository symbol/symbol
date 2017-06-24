#include "ModifyMultisigAccountMapper.h"
#include "plugins/mongo/coremongo/src/MongoTransactionPluginFactory.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/multisig/src/model/ModifyMultisigAccountTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		void StreamModification(bson_stream::array_context& context, model::CosignatoryModificationType type, const Key& key) {
			context
					<< bson_stream::open_document
						<< "type" << utils::to_underlying_type(type)
						<< "cosignatoryPublicKey" << ToBinary(key)
					<< bson_stream::close_document;
		}

		void StreamModifications(
				bson_stream::document& builder,
				const model::CosignatoryModification* pModification,
				size_t numModifications) {
			auto modificationsArray = builder << "modifications" << bson_stream::open_array;
			for (auto i = 0u; i < numModifications; ++i, ++pModification)
				StreamModification(modificationsArray, pModification->ModificationType, pModification->CosignatoryPublicKey);

			modificationsArray << bson_stream::close_array;
		}

		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "minRemovalDelta" << transaction.MinRemovalDelta
					<< "minApprovalDelta" << transaction.MinApprovalDelta;
			StreamModifications(builder, transaction.ModificationsPtr(), transaction.ModificationsCount);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateModifyMultisigAccountTransactionMongoPlugin() {
		using namespace catapult::model;
		return MongoTransactionPluginFactory::Create<ModifyMultisigAccountTransaction, EmbeddedModifyMultisigAccountTransaction>(
				StreamTransaction<ModifyMultisigAccountTransaction>,
				StreamTransaction<EmbeddedModifyMultisigAccountTransaction>);
	}
}}}
