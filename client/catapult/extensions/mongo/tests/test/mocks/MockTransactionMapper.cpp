/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "MockTransactionMapper.h"
#include "mongo/src/mappers/MapperUtils.h"

namespace catapult { namespace mocks {

	using namespace catapult::mongo::mappers;
	using MongoTransactionPlugin = mongo::MongoTransactionPlugin;
	using EmbeddedMongoTransactionPlugin = mongo::EmbeddedMongoTransactionPlugin;

	namespace {
		template<typename TTransaction, typename TDerivedTransaction, typename TPlugin>
		class MockMongoTransactionPluginT : public TPlugin {
		public:
			MockMongoTransactionPluginT(model::EntityType type, PluginOptionFlags options)
					: m_type(type)
					, m_options(options)
			{}

		public:
			model::EntityType type() const override {
				return m_type;
			}

			void streamTransaction(bson_stream::document& builder, const TTransaction& transaction) const override {
				const auto& mockTransaction = static_cast<const TDerivedTransaction&>(transaction);
				builder << "recipientPublicKey" << ToBinary(mockTransaction.RecipientPublicKey);

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
			MockMongoTransactionPlugin(model::EntityType type, PluginOptionFlags options, size_t numDependentDocuments)
					: MockMongoTransactionPluginT<model::Transaction, MockTransaction, MongoTransactionPlugin>(type, options)
					, m_numDependentDocuments(numDependentDocuments) {
				if (IsPluginOptionFlagSet(options, PluginOptionFlags::Not_Embeddable))
					return;

				m_pEmbeddedTransactionPlugin = std::make_unique<MockMongoTransactionPluginT<
						model::EmbeddedTransaction,
						EmbeddedMockTransaction,
						EmbeddedMongoTransactionPlugin>>(type, options);
			}

		public:
			std::vector<bsoncxx::document::value> extractDependentDocuments(
					const model::Transaction& transaction,
					const mongo::MongoTransactionMetadata&) const override {
				std::vector<bsoncxx::document::value> documents;
				for (auto i = 0u; i < m_numDependentDocuments; ++i) {
					auto document = bson_stream::document()
							<< "dd_counter" << static_cast<int32_t>(i)
							<< "aggregate_signer" << ToBinary(transaction.SignerPublicKey)
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

	std::unique_ptr<MongoTransactionPlugin> CreateMockTransactionMongoPlugin(model::EntityType type) {
		return std::make_unique<MockMongoTransactionPlugin>(type, PluginOptionFlags::Not_Embeddable, 0);
	}

	std::unique_ptr<MongoTransactionPlugin> CreateMockTransactionMongoPlugin(PluginOptionFlags options, size_t numDependentDocuments) {
		return std::make_unique<MockMongoTransactionPlugin>(MockTransaction::Entity_Type, options, numDependentDocuments);
	}
}}
