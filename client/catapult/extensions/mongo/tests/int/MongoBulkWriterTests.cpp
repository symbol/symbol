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

#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoTransactionPlugin.h"
#include "mongo/src/mappers/AccountStateMapper.h"
#include "mongo/src/mappers/TransactionMapper.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/StackLogger.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"
#include <mongocxx/pool.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoBulkWriterTests

	namespace {
		constexpr auto Transactions_Collection_Name = "transactions";
		constexpr auto Accounts_Collection_Name = "accounts";

		using AccountStates = std::unordered_set<std::shared_ptr<state::AccountState>>;
		using Transactions = std::vector<std::unique_ptr<model::Transaction>>;
		using TransactionElements = std::vector<model::TransactionElement>;

		int32_t GetDefaultEntityCount() {
			return test::GetStressIterationCount() ? 200'000 : 20'000;
		}

		auto CreateAccountStates(size_t count) {
			AccountStates accountStates;
			for (auto i = 0u; i < count; ++i)
				accountStates.insert(test::CreateAccountStateWithoutPublicKey(i + 1));

			return accountStates;
		}

		auto ExtractEverySecondAccount(const AccountStates& accountStates) {
			AccountStates extractedAccounts;
			auto i = 0u;
			for (const auto& pAccountState : accountStates) {
				if (0 == i++ % 2)
					extractedAccounts.insert(pAccountState);
			}

			return extractedAccounts;
		}

		auto ModifyAccounts(const AccountStates& accountStates) {
			for (auto& pAccountState : accountStates)
				pAccountState->Balances.credit(MosaicId(1111), Amount(123));
		}

		auto CreateTransactions(size_t count) {
			test::MutableTransactions transactions;
			for (auto i = 0u; i < count; ++i)
				transactions.push_back(mocks::CreateMockTransaction(10));

			return transactions;
		}

		auto CreateTransactionElements(const test::MutableTransactions& transactions) {
			TransactionElements transactionElements;
			for (const auto& pTransaction : transactions) {
				transactionElements.emplace_back(*pTransaction);
				auto& transactionElement = transactionElements.back();
				transactionElement.EntityHash = test::GenerateRandomByteArray<Hash256>();
				transactionElement.OptionalExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>();
			}

			return transactionElements;
		}

		auto CreateAccountDocument(const std::shared_ptr<state::AccountState>& pAccountState, uint32_t) {
			return mappers::ToDbModel(*pAccountState);
		}

		auto CreateDocument(
				const model::TransactionElement& transactionElement,
				Height height,
				uint32_t index,
				const MongoTransactionRegistry& registry) {
			auto metadata = MongoTransactionMetadata(transactionElement, height, index);
			return mappers::ToDbDocuments(transactionElement.Transaction, metadata, registry)[0];
		}

		auto CreateDocuments(
				const model::TransactionElement& transactionElement,
				Height height,
				uint32_t index,
				const MongoTransactionRegistry& registry) {
			std::vector<bsoncxx::document::value> documents;
			for (auto i = 1u; i <= 3; ++i)
				documents.push_back(CreateDocument(transactionElement, height, index, registry));

			return documents;
		}

		void AssertResult(
				int32_t numInserted,
				int32_t numMatched,
				int32_t numModified,
				int32_t numDeleted,
				int32_t numUpserted,
				const BulkWriteResult& result) {
			// Assert:
			EXPECT_EQ(numInserted, result.NumInserted);
			EXPECT_EQ(numMatched, result.NumMatched);
			EXPECT_EQ(numModified, result.NumModified);
			EXPECT_EQ(numDeleted, result.NumDeleted);
			EXPECT_EQ(numUpserted, result.NumUpserted);
		}

		class PerformanceContext {
		public:
			// the service thread pool has 8 threads since tests show that mongo performs best with this arrangement
			PerformanceContext() : PerformanceContext(static_cast<size_t>(GetDefaultEntityCount()))
			{}

			explicit PerformanceContext(size_t numEntities)
					: m_accountStates(CreateAccountStates(numEntities))
					, m_transactions(CreateTransactions(numEntities))
					, m_transactionElements(CreateTransactionElements(m_transactions))
					, m_pPool(test::CreateStartedIoThreadPool(test::Num_Default_Mongo_Test_Pool_Threads)) {
				test::PrepareDatabase(test::DatabaseName());
				m_pBulkWriter = MongoBulkWriter::Create(test::DefaultDbUri(), test::DatabaseName(), *m_pPool);
				m_connection = test::CreateDbConnection();
			}

			~PerformanceContext() {
				// wait for all pending thread pool work to complete
				m_pPool->join();
			}

		public:
			const auto& accountStates() const {
				return m_accountStates;
			}

			const auto& transactionElements() const {
				return m_transactionElements;
			}

			auto& bulkWriter() const {
				return *m_pBulkWriter;
			}

		public:
			void destroyBulkWriter() {
				m_pBulkWriter.reset();
			}

		private:
			AccountStates m_accountStates;
			test::MutableTransactions m_transactions;
			TransactionElements m_transactionElements;
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::shared_ptr<MongoBulkWriter> m_pBulkWriter;
			mongocxx::client m_connection;
		};
	}

	// region performance

	NO_STRESS_TEST(TEST_CLASS, InsertOneToOnePerformance) {
		// Arrange:
		PerformanceContext context;
		auto registry = test::CreateDefaultMongoTransactionRegistry();
		auto createDocument = [&registry](const auto& transactionElement, auto index) {
			return CreateDocument(transactionElement, Height(1), index, registry);
		};

		// Sanity:
		test::AssertCollectionSize(Transactions_Collection_Name, 0);

		// Act:
		utils::StackLogger stopwatch("InsertOneToOnePerformance", utils::LogLevel::warning);
		auto results = context.bulkWriter().bulkInsert<std::vector<model::TransactionElement>>(
				Transactions_Collection_Name,
				context.transactionElements(),
				createDocument).get();

		// Assert:
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		test::AssertCollectionSize(Transactions_Collection_Name, static_cast<uint64_t>(GetDefaultEntityCount()));
		AssertResult(GetDefaultEntityCount(), 0, 0, 0, 0, aggregateResult);
	}

	NO_STRESS_TEST(TEST_CLASS, InsertOneToManyPerformance) {
		// Arrange:
		PerformanceContext context;
		auto registry = test::CreateDefaultMongoTransactionRegistry();
		auto createDocuments = [&registry](const auto& transactionElement, auto index) {
			return CreateDocuments(transactionElement, Height(1), index, registry);
		};

		// Sanity:
		test::AssertCollectionSize(Transactions_Collection_Name, 0);

		// Act:
		utils::StackLogger stopwatch("InsertOneToManyPerformance", utils::LogLevel::warning);
		auto results = context.bulkWriter().bulkInsert<std::vector<model::TransactionElement>>(
				Transactions_Collection_Name,
				context.transactionElements(),
				createDocuments).get();

		// Assert: each entity is mapped to three documents
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		test::AssertCollectionSize(Transactions_Collection_Name, static_cast<uint64_t>(3 * GetDefaultEntityCount()));
		AssertResult(3 * GetDefaultEntityCount(), 0, 0, 0, 0, aggregateResult);
	}

	NO_STRESS_TEST(TEST_CLASS, UpsertPerformance) {
		// Arrange:
		// - insert half of the accounts into the db, then modify all accounts
		// - the subsequent bulk upsert thus results in
		//   1) half of the accounts being found in the db and being modified
		//   2) half of the accounts not being found and therefore inserted into the db
		PerformanceContext context;
		AccountStates extractedAccounts = ExtractEverySecondAccount(context.accountStates());
		context.bulkWriter().bulkUpsert<AccountStates>(
				Accounts_Collection_Name,
				extractedAccounts,
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		test::AssertCollectionSize(Accounts_Collection_Name, static_cast<uint64_t>(GetDefaultEntityCount() / 2));

		// Act:
		ModifyAccounts(context.accountStates());
		utils::StackLogger stopwatch("UpsertPerformance", utils::LogLevel::warning);
		auto results = context.bulkWriter().bulkUpsert<AccountStates>(
				Accounts_Collection_Name,
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Assert:
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		test::AssertCollectionSize(Accounts_Collection_Name, static_cast<uint64_t>(GetDefaultEntityCount()));
		AssertResult(0, GetDefaultEntityCount() / 2, GetDefaultEntityCount() / 2, 0, GetDefaultEntityCount() / 2, aggregateResult);
	}

	NO_STRESS_TEST(TEST_CLASS, DeleteOneToOnePerformance) {
		// Arrange:
		PerformanceContext context;
		context.bulkWriter().bulkUpsert<AccountStates>(
				Accounts_Collection_Name,
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		test::AssertCollectionSize(Accounts_Collection_Name, static_cast<uint64_t>(GetDefaultEntityCount()));

		// Act:
		utils::StackLogger stopwatch("DeleteOneToOnePerformance", utils::LogLevel::warning);
		auto results = context.bulkWriter().bulkDelete<AccountStates>(
				Accounts_Collection_Name,
				context.accountStates(),
				test::CreateFilter).get();

		// Assert:
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		test::AssertCollectionSize(Accounts_Collection_Name, 0);
		AssertResult(0, 0, 0, GetDefaultEntityCount(), 0, aggregateResult);
	}

	NO_STRESS_TEST(TEST_CLASS, DeleteOneToManyPerformance) {
		// Arrange:
		PerformanceContext context;
		context.bulkWriter().bulkUpsert<AccountStates>(
				Accounts_Collection_Name,
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		test::AssertCollectionSize(Accounts_Collection_Name, static_cast<uint64_t>(GetDefaultEntityCount()));

		// Act: simulate a multi-delete by passing in a single element vector and a select all filter
		//      the createFilter function is called once and returns a filter that matches all documents
		utils::StackLogger stopwatch("DeleteOneToManyPerformance", utils::LogLevel::warning);
		auto createSelectAllFilter = [](auto) { return document() << finalize; };
		auto results = context.bulkWriter().bulkDelete<std::vector<int>>(Accounts_Collection_Name, { 1 }, createSelectAllFilter).get();

		// Assert:
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		test::AssertCollectionSize(Accounts_Collection_Name, 0);
		AssertResult(0, 0, 0, GetDefaultEntityCount(), 0, aggregateResult);
	}

	// endregion

	// region traits

	namespace {
		template<typename T>
		bsoncxx::document::value CreateDocumentThrow(const T&, uint32_t) {
			throw std::runtime_error("unexpected call to CreateDocument");
		}

		template<typename T>
		std::vector<bsoncxx::document::value> CreateDocumentsThrow(const T&, uint32_t) {
			throw std::runtime_error("unexpected call to CreateDocuments");
		}

		template<typename T>
		bsoncxx::document::value CreateFilterThrow(const T&) {
			throw std::runtime_error("unexpected call to CreateFilter");
		}

		struct InsertOneToOneTraits {
			struct Capture {
				size_t NumCreateDocumentCalls = 0;
				const model::TransactionElement* pCreateDocumentElement = nullptr;
			};

			static const auto& GetElements(const PerformanceContext& context) {
				return context.transactionElements();
			}

			static auto Execute(
					MongoBulkWriter& writer,
					const TransactionElements& elements,
					const std::atomic_bool& blockFlag,
					Capture& capture) {
				auto createDocument = [&blockFlag, &capture](const auto& transactionElement, auto index) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateDocumentCalls;
					capture.pCreateDocumentElement = &transactionElement;

					auto registry = test::CreateDefaultMongoTransactionRegistry();
					return CreateDocument(transactionElement, Height(1), index, registry);
				};

				// Act:
				return writer.bulkInsert<TransactionElements>(Transactions_Collection_Name, elements, createDocument);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkInsert<TransactionElements>(
						Transactions_Collection_Name,
						{},
						CreateDocumentThrow<model::TransactionElement>);
			}

			static void AssertDelegation(
					const TransactionElements& elements,
					const Capture& capture,
					const BulkWriteResult& aggregateResult) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateDocumentCalls);
				EXPECT_EQ(&(*elements.cbegin()), capture.pCreateDocumentElement);
				AssertResult(1, 0, 0, 0, 0, aggregateResult);
			}
		};

		struct InsertOneToManyTraits {
			struct Capture {
				size_t NumCreateDocumentsCalls = 0;
				const model::TransactionElement* pCreateDocumentsElement = nullptr;
			};

			static const auto& GetElements(const PerformanceContext& context) {
				return context.transactionElements();
			}

			static auto Execute(
					MongoBulkWriter& writer,
					const TransactionElements& elements,
					const std::atomic_bool& blockFlag,
					Capture& capture) {
				auto createDocuments = [&blockFlag, &capture](const auto& transactionElement, auto index) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateDocumentsCalls;
					capture.pCreateDocumentsElement = &transactionElement;

					// add three documents per entity
					auto registry = test::CreateDefaultMongoTransactionRegistry();
					return CreateDocuments(transactionElement, Height(1), index, registry);
				};

				// Act:
				return writer.bulkInsert<TransactionElements>(Transactions_Collection_Name, elements, createDocuments);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkInsert<TransactionElements>(
						Transactions_Collection_Name,
						{},
						CreateDocumentsThrow<model::TransactionElement>);
			}

			static void AssertDelegation(
					const TransactionElements& elements,
					const Capture& capture,
					const BulkWriteResult& aggregateResult) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateDocumentsCalls);
				EXPECT_EQ(&(*elements.cbegin()), capture.pCreateDocumentsElement);
				AssertResult(3, 0, 0, 0, 0, aggregateResult); // 3 documents should have been inserted
			}
		};

		struct UpsertTraits {
			struct Capture {
				size_t NumCreateDocumentCalls = 0;
				const state::AccountState* pCreateDocumentAccountState = nullptr;

				size_t NumCreateFilterCalls = 0;
				const state::AccountState* pCreateFilterAccountState = nullptr;
			};

			static const auto& GetElements(const PerformanceContext& context) {
				return context.accountStates();
			}

			static auto Execute(
					MongoBulkWriter& writer,
					const AccountStates& accountStates,
					const std::atomic_bool& blockFlag,
					Capture& capture) {
				auto createDocument = [&blockFlag, &capture](const auto& pAccountState, auto) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateDocumentCalls;
					capture.pCreateDocumentAccountState = pAccountState.get();
					return mappers::ToDbModel(*pAccountState);
				};

				auto createFilter = [&blockFlag, &capture](const auto& pAccountState) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateFilterCalls;
					capture.pCreateFilterAccountState = pAccountState.get();
					return test::CreateFilter(pAccountState);
				};

				// Act:
				return writer.bulkUpsert<AccountStates>(Accounts_Collection_Name, accountStates, createDocument, createFilter);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkUpsert<AccountStates>(
						Accounts_Collection_Name,
						{},
						CreateDocumentThrow<AccountStates::value_type>,
						CreateFilterThrow<AccountStates::value_type>);
			}

			static void AssertDelegation(
					const AccountStates& accountStates,
					const Capture& capture,
					const BulkWriteResult& aggregateResult) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateDocumentCalls);
				EXPECT_EQ((*accountStates.cbegin()).get(), capture.pCreateDocumentAccountState);

				EXPECT_EQ(1u, capture.NumCreateFilterCalls);
				EXPECT_EQ((*accountStates.cbegin()).get(), capture.pCreateFilterAccountState);

				AssertResult(0, 0, 0, 0, 1, aggregateResult);
			}
		};

		struct DeleteTraits {
			struct Capture {
				size_t NumCreateFilterCalls = 0;
				const state::AccountState* pCreateFilterAccountState = nullptr;
			};

			static const auto& GetElements(const PerformanceContext& context) {
				return context.accountStates();
			}

			static auto Execute(
					MongoBulkWriter& writer,
					const AccountStates& accountStates,
					const std::atomic_bool& blockFlag,
					Capture& capture) {
				auto createFilter = [&blockFlag, &capture](const auto& pAccountState) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateFilterCalls;
					capture.pCreateFilterAccountState = pAccountState.get();
					return test::CreateFilter(pAccountState);
				};

				// Act:
				return writer.bulkDelete<AccountStates>(Accounts_Collection_Name, accountStates, createFilter);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkDelete<AccountStates>(Accounts_Collection_Name, {}, CreateFilterThrow<AccountStates::value_type>);
			}

			static void AssertDelegation(
					const AccountStates& accountStates,
					const Capture& capture,
					const BulkWriteResult& aggregateResult) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateFilterCalls);
				EXPECT_EQ((*accountStates.cbegin()).get(), capture.pCreateFilterAccountState);

				// - note that nothing was deleted because the db is empty
				AssertResult(0, 0, 0, 0, 0, aggregateResult);
			}
		};
	}

#define BULK_OPERATION_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_InsertOneToOne) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<InsertOneToOneTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_InsertOneToMany) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<InsertOneToManyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Upsert) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UpsertTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delete) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeleteTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region delegation

	BULK_OPERATION_TEST(BulkOperationDelegatesToLambdas) {
		// Arrange:
		PerformanceContext context(1);
		std::atomic_bool blockFlag(false);
		typename TTraits::Capture capture;

		// Act:
		auto results = TTraits::Execute(context.bulkWriter(), TTraits::GetElements(context), blockFlag, capture).get();

		// Assert:
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		TTraits::AssertDelegation(TTraits::GetElements(context), capture, aggregateResult);
	}

	// endregion

	// region shutdown

	BULK_OPERATION_TEST(FutureIsFulfilledEvenWhenWriterIsDestroyed) {
		// Arrange:
		PerformanceContext context(1);
		std::atomic_bool blockFlag(true);
		typename TTraits::Capture capture;
		thread::future<std::vector<thread::future<BulkWriteResult>>> future;
		{
			// Act: execute and destroy the writer
			CATAPULT_LOG(debug) << "starting async write";
			future = TTraits::Execute(context.bulkWriter(), TTraits::GetElements(context), blockFlag, capture);

			CATAPULT_LOG(debug) << "destroying writer";
			context.destroyBulkWriter();
		}

		// Sanity: the write is not complete
		EXPECT_FALSE(future.is_ready());
		CATAPULT_LOG(debug) << "unblocking writer";
		blockFlag = false;

		CATAPULT_LOG(debug) << "waiting for writer";
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(future.get()));

		// Assert: the write should have completed successfully
		TTraits::AssertDelegation(TTraits::GetElements(context), capture, aggregateResult);
	}

	// endregion

	// region zero entities

	BULK_OPERATION_TEST(BulkOperationCanProcessZeroEntities) {
		// Arrange:
		PerformanceContext context(1);

		// Act:
		auto results = TTraits::ExecuteZero(context.bulkWriter()).get();

		// Assert:
		auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		AssertResult(0, 0, 0, 0, 0, aggregateResult);
	}

	// endregion

	// region bulk writer exception

	TEST(TEST_CLASS, FutureExposesBulkWriteExceptions) {
		// Arrange:
		// - insert one account into the db, then attempt to insert the same account again
		// - this will result in a bulk writer exception due to a duplicate key (index on account address)
		PerformanceContext context(1);
		context.bulkWriter().bulkUpsert<AccountStates>(
				Accounts_Collection_Name,
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		test::AssertCollectionSize(Accounts_Collection_Name, 1);

		// - note that the statement will not throw since we are only calling get() on the future that holds the bulk write futures
		auto results = context.bulkWriter().bulkInsert<AccountStates>(
				Accounts_Collection_Name,
				context.accountStates(),
				CreateAccountDocument).get();

		// Act + Assert: the array of futures holds an exceptional future
		EXPECT_THROW(thread::get_all(std::move(results)), catapult_runtime_error);
	}

	// endregion
}}
