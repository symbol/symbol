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

#pragma once
#include "extensions/mongo/src/MongoBulkWriter.h"
#include "extensions/mongo/src/MongoStorageContext.h"
#include "extensions/mongo/src/MongoTransactionPlugin.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include <mongocxx/client.hpp>

namespace catapult {
	namespace model { struct TransactionInfo; }
	namespace mongo { class MongoTransactionRegistry; }
	namespace state { struct AccountState; }
}

namespace catapult { namespace test {

	/// Number of default mongo test thread pool threads.
	constexpr uint32_t Num_Default_Mongo_Test_Pool_Threads = 8;

	/// Gets the database name for tests.
	std::string DatabaseName();

	/// Gets the default db uri for tests.
	mongocxx::uri DefaultDbUri();

	/// Creates a mongo database connection.
	mongocxx::client CreateDbConnection();

	/// Resets the mongo database with name \a dbName by dropping and recreating it (in order to have empty collections).
	void ResetDatabase(const std::string& dbName);

	/// Prepares the mongo database with name \a dbName by
	/// 1) dropping and recreating it (in order to have empty collections).
	/// 2) adding indexes to the accounts and transactions collections.
	void PrepareDatabase(const std::string& dbName);

	/// Creates a filter for the given \a pAccountState.
	bsoncxx::document::value CreateFilter(const std::shared_ptr<state::AccountState>& pAccountState);

	/// Creates a default mongo storage context for database \a dbName using \a pool
	std::unique_ptr<mongo::MongoStorageContext> CreateDefaultMongoStorageContext(const std::string& dbName, thread::IoThreadPool& pool);

	/// Creates a default mongo transaction registry that supports mock transactions.
	mongo::MongoTransactionRegistry CreateDefaultMongoTransactionRegistry();

	/// Database mixin for preparing mongo database access.
	struct PrepareDatabaseMixin {
		/// Creates a database mixin around the test database.
		PrepareDatabaseMixin() {
			PrepareDatabase(DatabaseName());
		}
	};

	/// Asserts that the mongo database collection with name \a collectionName has size \a expectedSize.
	void AssertCollectionSize(const std::string& collectionName, uint64_t expectedSize);

	/// Asserts that \a collection contains \a expectedNumDependentDocuments dependent documents from \a transactionSigner.
	void AssertDependentDocuments(mongocxx::collection& collection, const Key& transactionSigner, size_t expectedNumDependentDocuments);

	/// Asserts that the collection with name \a collectionName contains \a expectedTransactionInfos and
	/// \a expectedNumDependentDocuments dependent documents.
	void AssertTransactions(
			const std::string& collectionName,
			const std::vector<model::TransactionInfo>& expectedTransactionInfos,
			size_t expectedNumDependentDocuments = 0);

	/// Enumerates the possible database initializations types.
	enum class DbInitializationType {
		/// Performs no initialization action.
		None,

		/// Drops the database.
		Reset,

		/// Drops the database, recreates it and creates indexes for the collections.
		Prepare
	};

	template<typename TStorage>
	using StorageFactory = std::function<std::unique_ptr<TStorage> (mongo::MongoStorageContext&, const mongo::MongoTransactionRegistry&)>;

	/// Creates a mongo storage around \a pTransactionPlugin using \a dbInitializationType for initializing the database,
	/// the specified error policy mode (\a errorPolicyMode) for inspecting errors and \a storageFactory to create the storage.
	template<typename TStorage>
	std::shared_ptr<TStorage> CreateMongoStorage(
			std::unique_ptr<mongo::MongoTransactionPlugin>&& pTransactionPlugin,
			DbInitializationType dbInitializationType,
			mongo::MongoErrorPolicy::Mode errorPolicyMode,
			const StorageFactory<TStorage>& storageFactory) {
		if (test::DbInitializationType::Reset == dbInitializationType)
			ResetDatabase(DatabaseName());
		else if (test::DbInitializationType::Prepare == dbInitializationType)
			PrepareDatabase(DatabaseName());

		auto pPool = utils::UniqueToShared(CreateStartedIoThreadPool(Num_Default_Mongo_Test_Pool_Threads));
		auto pWriter = mongo::MongoBulkWriter::Create(DefaultDbUri(), DatabaseName(), *pPool);
		auto pMongoContext = std::make_shared<mongo::MongoStorageContext>(DefaultDbUri(), DatabaseName(), pWriter, errorPolicyMode);

		auto pRegistry = std::make_shared<mongo::MongoTransactionRegistry>();
		pRegistry->registerPlugin(std::move(pTransactionPlugin));
		auto pStorage = utils::UniqueToShared(storageFactory(*pMongoContext, *pRegistry));
		return decltype(pStorage)(pStorage.get(), [pPool, pMongoContext, pRegistry, pStorage](const auto*) {});
	}
}}
