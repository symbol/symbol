#include "AggregateMapper.h"
#include "plugins/mongo/coremongo/src/MongoPluginManager.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		constexpr const model::AggregateTransaction& CastToDerivedType(const model::Transaction& transaction) {
			return static_cast<const model::AggregateTransaction&>(transaction);
		}

		void StreamCosignatures(bson_stream::document& builder, const model::Cosignature* pCosignature, size_t numCosignatures) {
			auto cosignaturesArray = builder << "cosignatures" << bson_stream::open_array;
			for (auto i = 0u; i < numCosignatures; ++i) {
				cosignaturesArray
						<< bson_stream::open_document
							<< "signer" << ToBinary(pCosignature->Signer)
							<< "signature" << ToBinary(pCosignature->Signature)
						<< bson_stream::close_document;
				++pCosignature;
			}

			cosignaturesArray << bson_stream::close_array;
		}

		class AggregateTransactionPlugin : public MongoTransactionPlugin {
		public:
			explicit AggregateTransactionPlugin(const MongoTransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			model::EntityType type() const override {
				return model::EntityType::Aggregate;
			}

			void streamTransaction(bson_stream::document& builder, const model::Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);
				StreamCosignatures(builder, aggregate.CosignaturesPtr(), aggregate.CosignaturesCount());
			}

			std::vector<bsoncxx::document::value> extractDependentDocuments(
					const model::Transaction& transaction,
					const MongoTransactionMetadata& metadata) const override {
				const auto& aggregate = CastToDerivedType(transaction);

				auto i = 0;
				std::vector<bsoncxx::document::value> documents;
				for (const auto& subTransaction : aggregate.Transactions()) {
					const auto& plugin = m_transactionRegistry.findPlugin(subTransaction.Type)->embeddedPlugin();

					// transaction metadata
					bson_stream::document builder;
					builder << "meta"
							<< bson_stream::open_document
								<< "height" << ToInt64(metadata.Height)
								<< "aggregateId" << metadata.ObjectId
								<< "index" << static_cast<int32_t>(i++)
							<< bson_stream::close_document;

					// transaction data
					builder << "transaction" << bson_stream::open_document;
					StreamEmbeddedEntity(builder, subTransaction);
					plugin.streamTransaction(builder, subTransaction);
					builder << bson_stream::close_document;

					documents.push_back(builder << bson_stream::finalize);
				}

				return documents;
			}

			bool supportsEmbedding() const override {
				return false;
			}

			const EmbeddedMongoTransactionPlugin& embeddedPlugin() const override {
				CATAPULT_THROW_RUNTIME_ERROR("aggregate transaction is not embeddable");
			}

		private:
			const MongoTransactionRegistry& m_transactionRegistry;
		};
	}

	std::unique_ptr<MongoTransactionPlugin> CreateAggregateTransactionMongoPlugin(const MongoTransactionRegistry& transactionRegistry) {
		return std::make_unique<AggregateTransactionPlugin>(transactionRegistry);
	}
}}}

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::plugins::MongoPluginManager& manager) {
	manager.addTransactionSupport(catapult::mongo::plugins::CreateAggregateTransactionMongoPlugin(manager.transactionRegistry()));
}
