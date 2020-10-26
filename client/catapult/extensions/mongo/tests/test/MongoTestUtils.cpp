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

#include "MongoTestUtils.h"
#include "MapperTestUtils.h"
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoPluginLoader.h"
#include "mongo/src/MongoStorageContext.h"
#include "mongo/src/MongoTransactionMetadata.h"
#include "mongo/src/MongoTransactionPlugin.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/state/AccountState.h"
#include "mongo/tests/test/mocks/MockTransactionMapper.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace test {

	std::string DatabaseName() {
		return "test-db";
	}

	mongocxx::uri DefaultDbUri() {
#ifndef CATAPULT_TEST_DB_URL
		return mongocxx::uri("mongodb://127.0.0.1:27017");
#else
		return mongocxx::uri(CATAPULT_TEST_DB_URL);
#endif
	}

	mongocxx::client CreateDbConnection() {
		return mongocxx::client(DefaultDbUri());
	}

	void ResetDatabase(const std::string& dbName) {
		mongocxx::instance::current();
		auto connection = CreateDbConnection();
		connection[dbName].drop();
	}

	void PrepareDatabase(const std::string& dbName) {
		mongocxx::instance::current();
		auto connection = CreateDbConnection();
		connection[dbName].drop();
		auto database = connection[dbName];

		// indexes for the 'accounts' collection
		auto accounts = database["accounts"];
		auto accountsAddressIndex = document() << "account.address" << 1 << finalize;
		auto publicKeyIndex = document() << "account.publicKey" << 1 << finalize;
		accounts.create_index(accountsAddressIndex.view(), mongocxx::options::index().unique(true));
		accounts.create_index(publicKeyIndex.view(), mongocxx::options::index());

		// indexes for the 'finalizedBlocks' collection
		auto finalizedBlocks = database["finalizedBlocks"];
		auto finalizationRoundIndex = document() << "block.finalizationEpoch" << -1 << "block.finalizationPoint" << -1 << finalize;
		finalizedBlocks.create_index(finalizationRoundIndex.view(), mongocxx::options::index().unique(true));

		// indexes for the 'transactions' collection
		auto transactions = database["transactions"];
		auto deadlineIndex = document() << "transaction.deadline" << -1 << finalize;
		auto signerIndex = document() << "transaction.signerPublicKey" << 1 << "_id" << -1 << finalize;
		auto recipientIndex = document() << "transaction.recipient" << 1 << "_id" << -1 << finalize;
		auto heightIndex = document() << "meta.height" << -1 << finalize;
		transactions.create_index(deadlineIndex.view(), mongocxx::options::index());
		transactions.create_index(signerIndex.view(), mongocxx::options::index());
		transactions.create_index(recipientIndex.view(), mongocxx::options::index());
		transactions.create_index(heightIndex.view(), mongocxx::options::index());

		// indexes for the 'unconfirmedTransactions' collection
		auto unconfirmedTransactions = database["unconfirmedTransactions"];
		auto hashIndex = document() << "meta.hash" << 1 << finalize;
		unconfirmedTransactions.create_index(hashIndex.view(), mongocxx::options::index().unique(true));
	}

	bsoncxx::document::value CreateFilter(const std::shared_ptr<state::AccountState>& pAccountState) {
		auto filter = document()
				<< "account.address" << open_document
					<< "$eq" << mongo::mappers::ToBinary(pAccountState->Address)
				<< close_document
				<< finalize;
		return filter;
	}

	std::unique_ptr<mongo::MongoStorageContext> CreateDefaultMongoStorageContext(const std::string& dbName, thread::IoThreadPool& pool) {
		auto pWriter = mongo::MongoBulkWriter::Create(DefaultDbUri(), dbName, pool);
		return std::make_unique<mongo::MongoStorageContext>(DefaultDbUri(), dbName, pWriter, mongo::MongoErrorPolicy::Mode::Strict);
	}

	mongo::MongoTransactionRegistry CreateDefaultMongoTransactionRegistry() {
		auto registry = mongo::MongoTransactionRegistry();
		registry.registerPlugin(mocks::CreateMockTransactionMongoPlugin());
		return registry;
	}

	namespace {
		auto Count(mongocxx::collection& collection, bsoncxx::document::view filter) {
			return static_cast<size_t>(collection.count_documents(filter));
		}
	}

	void AssertCollectionSize(const std::string& collectionName, uint64_t expectedSize) {
		auto connection = CreateDbConnection();
		auto database = connection[DatabaseName()];
		auto filter = document() << finalize;
		auto collection = database[collectionName];
		EXPECT_EQ(expectedSize, Count(collection, filter.view())) << "for collection " << collectionName;
	}

	// region mapped mock transaction asserts

	void AssertDependentDocuments(mongocxx::collection& collection, const Key& transactionSigner, size_t expectedNumDependentDocuments) {
		// sort by counter and filter by (aggregate) signer
		mongocxx::options::find options;
		options.sort(document() << "dd_counter" << 1 << finalize);
		auto filter = document() << "aggregate_signer" << mongo::mappers::ToBinary(transactionSigner) << finalize;
		ASSERT_EQ(expectedNumDependentDocuments, Count(collection, filter.view()));

		auto cursor = collection.find(filter.view(), options);
		auto iter = cursor.begin();
		for (auto i = 0u; i < expectedNumDependentDocuments; ++i) {
			// 'dd_counter' is save as an int so retrieve as an int
			EXPECT_EQ(static_cast<int32_t>(i), test::GetInt32(*iter, "dd_counter")) << "dependent document at " << i;
			++iter;
		}
	}

	void AssertTransactions(
			const std::string& collectionName,
			const std::vector<model::TransactionInfo>& expectedTransactionInfos,
			size_t expectedNumDependentDocuments) {
		auto connection = test::CreateDbConnection();
		auto database = connection[test::DatabaseName()];
		auto collection = database[collectionName];

		// MockTransactionMongoPlugin creates dependent documents with 'dd_counter' property, so documents without that property
		// should be transactions
		auto txFilter = document() << "dd_counter" << open_document << "$exists" << false << close_document << finalize;

		// sort the retrieved transactions by deadline since the bulk writer doesn't
		// guarantee any ordering while inserting the documents
		mongocxx::options::find options;
		options.sort(document() << "transaction.deadline" << 1 << finalize);

		// Assert: check collection size
		EXPECT_EQ(expectedTransactionInfos.size(), Count(collection, txFilter.view()));
		EXPECT_EQ((1 + expectedNumDependentDocuments) * expectedTransactionInfos.size(), Count(collection, document() << finalize));

		auto txCursor = collection.find(txFilter.view(), options);
		auto iter = txCursor.begin();
		for (const auto& expectedTransactionInfo : expectedTransactionInfos) {
			const auto& expectedTransaction = static_cast<const mocks::MockTransaction&>(*expectedTransactionInfo.pEntity);
			const auto& transactionDocument = (*iter)["transaction"].get_document().value;
			AssertEqualMockTransactionData(expectedTransaction, transactionDocument);

			const auto& transactionMeta = (*iter)["meta"].get_document().value;
			auto expectedMetadata = mongo::MongoTransactionMetadata(expectedTransactionInfo);
			AssertEqualTransactionMetadata(expectedMetadata, transactionMeta);

			// - check dependent documents
			AssertDependentDocuments(collection, expectedTransactionInfo.pEntity->SignerPublicKey, expectedNumDependentDocuments);
			++iter;
		}
	}

	// endregion
}}
