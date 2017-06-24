#include "MockTransactionMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"

namespace catapult { namespace mocks {
	using namespace catapult::mongo::mappers;
	using MongoTransactionPlugin = mongo::plugins::MongoTransactionPlugin;
	using EmbeddedMongoTransactionPlugin = mongo::plugins::EmbeddedMongoTransactionPlugin;

	namespace {
		template<typename TTransaction, typename TDerivedTransaction, typename TPlugin>
		class MockMongoTransactionPluginT : public TPlugin {
		public:
			explicit MockMongoTransactionPluginT(model::EntityType type, PluginOptionFlags options)
					: m_type(type)
					, m_options(options)
			{}

		public:
			model::EntityType type() const override {
				return m_type;
			}

			void streamTransaction(bson_stream::document& builder, const TTransaction& transaction) const override {
				const auto& mockTransaction = static_cast<const TDerivedTransaction&>(transaction);
				builder << "recipient" << ToBinary(mockTransaction.Recipient);

				if (mockTransaction.Data.Size)
					builder << "data" << ToBinary(mockTransaction.DataPtr(), mockTransaction.Data.Size);
			}

		private:
			model::EntityType m_type;
			PluginOptionFlags m_options;
		};

		class MockMongoTransactionPlugin
				: public MockMongoTransactionPluginT<model::Transaction, MockTransaction, MongoTransactionPlugin> {
		public:
			explicit MockMongoTransactionPlugin(model::EntityType type, PluginOptionFlags options, size_t numDependentDocuments)
					: MockMongoTransactionPluginT<model::Transaction, MockTransaction, MongoTransactionPlugin>(type, options)
					, m_numDependentDocuments(numDependentDocuments) {
				if (IsPluginOptionFlagSet(options, PluginOptionFlags::Not_Embeddable))
					return;

				m_pEmbeddedTransactionPlugin = std::make_unique<MockMongoTransactionPluginT<
						model::EmbeddedEntity,
						EmbeddedMockTransaction,
						EmbeddedMongoTransactionPlugin>>(type, options);
			}

		public:
			std::vector<bsoncxx::document::value> extractDependentDocuments(
					const model::Transaction& transaction,
					const mongo::plugins::MongoTransactionMetadata&) const override {
				std::vector<bsoncxx::document::value> documents;
				for (auto i = 0u; i < m_numDependentDocuments; ++i) {
					auto document = bson_stream::document{}
							<< "dd_counter" << static_cast<int32_t>(i)
							<< "aggregate_signer" << ToBinary(transaction.Signer)
							<< bson_stream::finalize;
					documents.push_back(document);
				}

				return documents;
			}

			bool supportsEmbedding() const override {
				return !!m_pEmbeddedTransactionPlugin;
			}

			const EmbeddedMongoTransactionPlugin& embeddedPlugin() const override {
				if (!m_pEmbeddedTransactionPlugin)
					CATAPULT_THROW_RUNTIME_ERROR("mock transaction is not embeddable");

				return *m_pEmbeddedTransactionPlugin;
			}

		private:
			size_t m_numDependentDocuments;
			std::unique_ptr<EmbeddedMongoTransactionPlugin> m_pEmbeddedTransactionPlugin;
		};
	}

	std::unique_ptr<MongoTransactionPlugin> CreateMockTransactionMongoPlugin(int type) {
		return std::make_unique<MockMongoTransactionPlugin>(static_cast<model::EntityType>(type), PluginOptionFlags::Not_Embeddable, 0);
	}

	std::unique_ptr<MongoTransactionPlugin> CreateMockTransactionMongoPlugin(PluginOptionFlags options, size_t numDependentDocuments) {
		return std::make_unique<MockMongoTransactionPlugin>(
				static_cast<model::EntityType>(MockTransaction::Entity_Type),
				options,
				numDependentDocuments);
	}
}}
