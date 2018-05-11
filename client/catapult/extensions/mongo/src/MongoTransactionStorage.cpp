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

#include "MongoTransactionStorage.h"
#include "MongoBulkWriter.h"
#include "MongoTransactionMetadata.h"
#include "mappers/MapperUtils.h"
#include "mappers/TransactionMapper.h"
#include <boost/optional.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		class DefaultMongoTransactionStorage final : public cache::UtChangeSubscriber {
		public:
			explicit DefaultMongoTransactionStorage(
					MongoStorageContext& context,
					const MongoTransactionRegistry& transactionRegistry,
					const std::string& collectionName)
					: m_context(context)
					, m_transactionRegistry(transactionRegistry)
					, m_collectionName(collectionName)
					, m_database(m_context.createDatabaseConnection())
			{}

		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				commitInserts(transactionInfos);
			}

			void notifyRemoves(const TransactionInfos& transactionInfos) override {
				commitDeletes(transactionInfos);
			}

			void flush() override {
				// empty because data is committed in notifyAdds and notifyRemoves
			}

		private:
			void commitInserts(const TransactionInfos& addedTransactionInfos) {
				// note that unconfirmed transactions don't have height and block index metadata
				const auto& registry = m_transactionRegistry;
				std::atomic<size_t> numTotalTransactionDocuments(0);
				auto createDocuments = [&registry, &numTotalTransactionDocuments](const auto& transactionInfo, auto) {
					auto metadata = MongoTransactionMetadata(transactionInfo);
					auto documents = mappers::ToDbDocuments(*transactionInfo.pEntity, metadata, registry);
					numTotalTransactionDocuments += documents.size();
					return documents;
				};
				auto results = m_context.bulkWriter().bulkInsert(m_collectionName, addedTransactionInfos, createDocuments).get();
				auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
				auto numInsertedDocuments = mappers::ToUint32(aggregate.NumInserted);
				if (addedTransactionInfos.size() > numInsertedDocuments || numTotalTransactionDocuments != numInsertedDocuments) {
					CATAPULT_THROW_RUNTIME_ERROR_2(
							"insert unconfirmed transactions failed: insert count mismatch (expected, actual)",
							addedTransactionInfos.size(),
							numInsertedDocuments);
				}
			}

			void commitDeletes(const TransactionInfos& removedTransactionInfos) {
				auto createFilter = [](const auto& info) {
					return document()
							<< "$or"
								<< open_array
									<< open_document << "meta.hash" << mappers::ToBinary(info.EntityHash) << close_document
									<< open_document << "meta.aggregateHash" << mappers::ToBinary(info.EntityHash) << close_document
								<< close_array
							<< finalize;
				};
				auto results = m_context.bulkWriter().bulkDelete(m_collectionName, removedTransactionInfos, createFilter).get();
				auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
				auto numDeleted = mappers::ToUint32(aggregate.NumDeleted);
				if (removedTransactionInfos.size() > numDeleted) {
					CATAPULT_THROW_RUNTIME_ERROR_2(
							"delete unconfirmed transactions failed: delete count mismatch (min expected, actual)",
							removedTransactionInfos.size(),
							numDeleted);
				}
			}

		private:
			MongoStorageContext& m_context;
			const MongoTransactionRegistry& m_transactionRegistry;
			std::string m_collectionName;
			MongoDatabase m_database;
		};
	}

	std::unique_ptr<cache::UtChangeSubscriber> CreateMongoTransactionStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry,
			const std::string& collectionName) {
		return std::make_unique<DefaultMongoTransactionStorage>(context, transactionRegistry, collectionName);
	}
}}
