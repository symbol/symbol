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
#include "MongoTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/mocks/MockTransactionMapper.h"
#include "tests/test/core/TransactionInfoTestUtils.h"

namespace catapult { namespace test {

	/// Group of tests for testing mongo transaction storages.
	template<typename TContext>
	class MongoTransactionStorageTests {
	private:
		static constexpr size_t Num_Transactions = 10;

		static void RemoveEverySecondTransactionInfo(std::vector<model::TransactionInfo>& transactionInfos, TContext& context) {
			auto i = 0u;
			for (auto iter = transactionInfos.begin(); transactionInfos.end() != iter;) {
				if (1 == ++i % 2) {
					context.removeTransaction(*iter);
					iter = transactionInfos.erase(iter);
					continue;
				}

				++iter;
			}
		}

		// region saving transactions

	public:
		static void CanSaveSingleTransaction() {
			// Arrange:
			TContext context(1);

			// Act:
			context.saveTransaction(context.transactionInfos().back());

			// Assert:
			AssertTransactions(context.collectionName(), context.transactionInfos());
		}

		static void CanSaveSingleTransactionWithDependentDocuments() {
			// Arrange:
			TContext context(1, mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, 3));

			// Act:
			context.saveTransaction(context.transactionInfos().back());

			// Assert:
			AssertTransactions(context.collectionName(), context.transactionInfos(), 3);
		}

	private:
		template<typename TAddAll>
		static void CanSaveMultipleTransactions(TAddAll addAll) {
			// Arrange:
			TContext context(Num_Transactions);

			// Act:
			addAll(context);

			// Assert:
			AssertTransactions(context.collectionName(), context.transactionInfos());
		}

	public:
		static void CanSaveMultipleTransactions_SingleCall() {
			// Assert:
			CanSaveMultipleTransactions([](auto& context) {
				context.saveTransactions(test::CopyTransactionInfosToSet(context.transactionInfos()));
			});
		}

		static void CanSaveMultipleTransactions_MultipleCalls() {
			// Assert:
			CanSaveMultipleTransactions([](auto& context) {
				for (const auto& transactionInfo : context.transactionInfos())
					context.saveTransaction(transactionInfo);
			});
		}

		// endregion

		// region removing transactions

	public:
		static void CanRemoveSingleTransaction() {
			// Arrange:
			TContext context(Num_Transactions);
			context.seedDatabase();

			// Act:
			context.removeTransaction(context.transactionInfos()[4]);
			context.transactionInfos().erase(context.transactionInfos().cbegin() + 4);

			// Assert:
			AssertCollectionSize(context.collectionName(), Num_Transactions - 1);
			AssertTransactions(context.collectionName(), context.transactionInfos());
		}

		static void CanRemoveSingleTransactionWithDependentDocuments() {
			using namespace bsoncxx::builder::stream;

			// Arrange:
			TContext context(Num_Transactions);
			context.seedDatabase();

			// - add dependent documents
			{
				auto connection = CreateDbConnection();
				auto collection = connection[DatabaseName()][context.collectionName()];
				auto hash = context.transactionInfos()[4].EntityHash;

				for (auto i = 0; i < 3; ++i) {
					auto dependentDocument = document()
							<< "meta" << open_document << "aggregateHash" << mongo::mappers::ToBinary(hash) << close_document
							<< "transaction" << open_document << "i" << i << close_document
							<< finalize;

					collection.insert_one(dependentDocument.view());
				}
			}

			// Sanity: the dependent documents were added to the storage
			AssertCollectionSize(context.collectionName(), Num_Transactions + 3);

			// Act:
			context.removeTransaction(context.transactionInfos()[4]);
			context.transactionInfos().erase(context.transactionInfos().cbegin() + 4);

			// Assert: all dependent documents were also deleted
			AssertCollectionSize(context.collectionName(), Num_Transactions - 1);
			AssertTransactions(context.collectionName(), context.transactionInfos());
		}

	private:
		template<typename TRemoveAll>
		static void CanRemoveMultipleTransactions(TRemoveAll removeAll) {
			// Arrange:
			TContext context(Num_Transactions);
			context.seedDatabase();

			std::vector<model::TransactionInfo> removedTransactionInfos;
			for (auto i = 0u; i < Num_Transactions / 2; ++i) {
				removedTransactionInfos.push_back(context.transactionInfos()[i].copy());
				context.transactionInfos().erase(context.transactionInfos().cbegin() + i);
			}

			// Act: remove all entries that had an even index
			removeAll(context, removedTransactionInfos);

			// Assert:
			AssertCollectionSize(context.collectionName(), Num_Transactions / 2);
			AssertTransactions(context.collectionName(), context.transactionInfos());
		}

	public:
		static void CanRemoveMultipleTransactions_SingleCall() {
			// Assert:
			CanRemoveMultipleTransactions([](auto& context, const auto& transactionInfos) {
				context.removeTransactions(test::CopyTransactionInfosToSet(transactionInfos));
			});
		}

		static void CanRemoveMultipleTransactions_MultipleCalls() {
			// Assert:
			CanRemoveMultipleTransactions([](auto& context, const auto& transactionInfos) {
				for (const auto& transactionInfo : transactionInfos)
					context.removeTransaction(transactionInfo);
			});
		}

		// endregion

		// region flush

	public:
		static void FlushDoesNotAddOrRemoveTransactions() {
			// Arrange:
			TContext context(Num_Transactions);
			context.seedDatabase();
			auto additonalTransactionInfos = CreateTransactionInfos(3, [](auto i) { return Timestamp(1234 + i); });

			// Sanity:
			test::AssertCollectionSize(context.collectionName(), Num_Transactions);

			// - remove every second transaction (ids 20, 40, 60, 80, 100 left)
			auto& transactionInfos = context.transactionInfos();
			RemoveEverySecondTransactionInfo(transactionInfos, context);

			// - add a few transactions
			for (auto& transactionInfo : additonalTransactionInfos) {
				context.saveTransaction(transactionInfo);
				transactionInfos.push_back(std::move(transactionInfo));
			}

			// Sanity:
			test::AssertCollectionSize(context.collectionName(), 5 + 3);
			test::AssertTransactions(context.collectionName(), transactionInfos);

			// Act:
			context.flush();

			// Assert: flush does not change any transactions (all changes have been previously committed)
			test::AssertCollectionSize(context.collectionName(), 5 + 3);
			test::AssertTransactions(context.collectionName(), transactionInfos);
		}

		// endregion
	};

#define MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, TEST_NAME) \
	TEST(Mongo##TRANSACTION_STORAGE_CLASS##Tests, TEST_NAME) { \
		test::MongoTransactionStorageTests<TRANSACTION_STORAGE_CLASS##Context>::TEST_NAME(); \
	} \

#define DEFINE_MONGO_TRANSACTION_STORAGE_SAVE_TESTS(TRANSACTION_STORAGE_CLASS) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanSaveSingleTransaction) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanSaveSingleTransactionWithDependentDocuments) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanSaveMultipleTransactions_SingleCall) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanSaveMultipleTransactions_MultipleCalls)

#define DEFINE_MONGO_TRANSACTION_STORAGE_REMOVE_TESTS(TRANSACTION_STORAGE_CLASS) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanRemoveSingleTransaction) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanRemoveSingleTransactionWithDependentDocuments) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanRemoveMultipleTransactions_SingleCall) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, CanRemoveMultipleTransactions_MultipleCalls)

#define DEFINE_MONGO_TRANSACTION_STORAGE_FLUSH_TESTS(TRANSACTION_STORAGE_CLASS) \
	MAKE_MONGO_TRANSACTION_STORAGE_TEST(TRANSACTION_STORAGE_CLASS, FlushDoesNotAddOrRemoveTransactions)
}}
