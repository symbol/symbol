/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "AggregateMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
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
							<< "version" << static_cast<int64_t>(pCosignature->Version)
							<< "signerPublicKey" << ToBinary(pCosignature->SignerPublicKey)
							<< "signature" << ToBinary(pCosignature->Signature)
						<< bson_stream::close_document;
				++pCosignature;
			}

			cosignaturesArray << bson_stream::close_array;
		}

		class AggregateTransactionPlugin : public MongoTransactionPlugin {
		public:
			AggregateTransactionPlugin(const MongoTransactionRegistry& transactionRegistry, model::EntityType transactionType)
					: m_transactionRegistry(transactionRegistry)
					, m_transactionType(transactionType)
			{}

		public:
			model::EntityType type() const override {
				return m_transactionType;
			}

			void streamTransaction(bson_stream::document& builder, const model::Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);
				builder << "transactionsHash" << ToBinary(aggregate.TransactionsHash);
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
					builder
							<< "meta" << bson_stream::open_document
								<< "height" << ToInt64(metadata.Height)
								<< "aggregateHash" << ToBinary(metadata.EntityHash)
								<< "aggregateId" << metadata.ObjectId
								<< "index" << static_cast<int32_t>(i++)
							<< bson_stream::close_document;

					// transaction data
					builder << "transaction" << bson_stream::open_document;
					StreamEmbeddedTransaction(builder, subTransaction);
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
			model::EntityType m_transactionType;
		};
	}

	std::unique_ptr<MongoTransactionPlugin> CreateAggregateTransactionMongoPlugin(
			const MongoTransactionRegistry& transactionRegistry,
			model::EntityType transactionType) {
		return std::make_unique<AggregateTransactionPlugin>(transactionRegistry, transactionType);
	}
}}}
