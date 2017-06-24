#include "plugins/mongo/coremongo/src/mappers/AccountStateMapper.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoTransactionPlugin.h"
#include "plugins/mongo/coremongo/src/mappers/TransactionMapper.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/StackLogger.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/pool.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoBulkWriterTests

	namespace {
#ifdef STRESS
		constexpr size_t Num_Entities = 200000;
#else
		constexpr size_t Num_Entities = 20000;
#endif

		using AccountStates = std::unordered_set<std::shared_ptr<state::AccountState>>;
		using Transactions = std::vector<std::unique_ptr<model::Transaction>>;
		using TransactionElements = std::vector<model::TransactionElement>;

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
				pAccountState->Balances.credit(Xem_Id, Amount(123));
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
				transactionElements.back().EntityHash = test::GenerateRandomData<Hash256_Size>();
			}

			return transactionElements;
		}

		auto CreateAccountDocument(const std::shared_ptr<state::AccountState>& pAccountState, uint32_t) {
			return mappers::ToDbModel(*pAccountState);
		}

		auto CreateDocument(
				const model::TransactionElement& txElement,
				Height height,
				uint32_t index,
				const MongoTransactionRegistry& registry) {
			auto metadata = MongoTransactionMetadata(txElement.EntityHash, txElement.MerkleComponentHash, height, index);
			return mappers::ToDbDocuments(txElement.Transaction, metadata, registry)[0];
		}

		auto CreateDocuments(
				const model::TransactionElement& txElement,
				Height height,
				uint32_t index,
				const MongoTransactionRegistry& registry) {
			std::vector<bsoncxx::document::value> documents;
			for (auto i = 1u; i <= 3u; ++i)
				documents.push_back(CreateDocument(txElement, height, index, registry));

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
			PerformanceContext(size_t numEntities = Num_Entities)
					: m_accountStates(CreateAccountStates(numEntities))
					, m_transactions(CreateTransactions(numEntities))
					, m_transactionElements(CreateTransactionElements(m_transactions))
					, m_pPool(test::CreateStartedIoServiceThreadPool(8)) {
				test::PrepareDatabase(test::DatabaseName());
				m_pBulkWriter = MongoBulkWriter::Create(test::DefaultDbUri(), test::DatabaseName(), m_pPool);
				m_connection = test::CreateDbConnection();
			}

			~PerformanceContext() {
				// wait for all pending threadpool work to complete
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

		public:
			size_t numEntitiesInCollection(const std::string& collectionName) {
				auto database = m_connection[test::DatabaseName()];
				auto collection = database[collectionName];
				auto filter = bsoncxx::builder::stream::document{} << bsoncxx::builder::stream::finalize;
				return static_cast<size_t>(collection.count(filter.view()));
			}

		private:
			AccountStates m_accountStates;
			test::MutableTransactions m_transactions;
			TransactionElements m_transactionElements;
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			std::shared_ptr<MongoBulkWriter> m_pBulkWriter;
			mongocxx::client m_connection;
		};
	}

	// region performance

	NO_STRESS_TEST(TEST_CLASS, InsertOneToOnePerformance) {
		// Arrange:
		PerformanceContext context;
		auto pRegistry = test::CreateDefaultMongoTransactionRegistry();
		auto createDocument = [pRegistry](const auto& txElement, auto index) {
			return CreateDocument(txElement, Height(1), index, *pRegistry);
		};

		// Sanity:
		EXPECT_EQ(0u, context.numEntitiesInCollection("transactions"));

		// Act:
		utils::StackLogger stopwatch("InsertOneToOnePerformance", utils::LogLevel::Warning);
		auto results = context.bulkWriter().bulkInsert<std::vector<model::TransactionElement>>(
				"transactions",
				context.transactionElements(),
				createDocument).get();

		// Assert:
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		EXPECT_EQ(Num_Entities, context.numEntitiesInCollection("transactions"));
		AssertResult(Num_Entities, 0, 0, 0, 0, aggregate);
	}

	NO_STRESS_TEST(TEST_CLASS, InsertOneToManyPerformance) {
		// Arrange:
		PerformanceContext context;
		auto pRegistry = test::CreateDefaultMongoTransactionRegistry();
		auto createDocuments = [pRegistry](const auto& txElement, auto index) {
			return CreateDocuments(txElement, Height(1), index, *pRegistry);
		};

		// Sanity:
		EXPECT_EQ(0u, context.numEntitiesInCollection("transactions"));

		// Act:
		utils::StackLogger stopwatch("InsertOneToManyPerformance", utils::LogLevel::Warning);
		auto results = context.bulkWriter().bulkInsert<std::vector<model::TransactionElement>>(
				"transactions",
				context.transactionElements(),
				createDocuments).get();

		// Assert: each entity is mapped to three documents
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		EXPECT_EQ(3 * Num_Entities, context.numEntitiesInCollection("transactions"));
		AssertResult(3 * Num_Entities, 0, 0, 0, 0, aggregate);
	}

	NO_STRESS_TEST(TEST_CLASS, UpsertPerformance) {
		// Arrange:
		// - insert half of the accounts into the db, then modify all accounts
		// - the subsequent bulk upsert thus results in
		//   1) half of the accounts being found in the db and being modified
		//   2) half of the accounts not being found and therefore inserted into the db
		PerformanceContext context;
		AccountStates extractedAccounts = ExtractEverySecondAccount(context.accountStates());
		context.bulkWriter().bulkUpsert<AccountStates>("accounts", extractedAccounts, CreateAccountDocument, test::CreateFilter).get();

		// Sanity:
		EXPECT_EQ(Num_Entities / 2, context.numEntitiesInCollection("accounts"));

		// Act:
		ModifyAccounts(context.accountStates());
		utils::StackLogger stopwatch("UpsertPerformance", utils::LogLevel::Warning);
		auto results = context.bulkWriter().bulkUpsert<AccountStates>(
				"accounts",
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Assert:
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		EXPECT_EQ(Num_Entities, context.numEntitiesInCollection("accounts"));
		AssertResult(0, Num_Entities / 2, Num_Entities / 2, 0, Num_Entities / 2, aggregate);
	}

	NO_STRESS_TEST(TEST_CLASS, DeleteOneToOnePerformance) {
		// Arrange:
		PerformanceContext context;
		context.bulkWriter().bulkUpsert<AccountStates>(
				"accounts",
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		EXPECT_EQ(Num_Entities, context.numEntitiesInCollection("accounts"));

		// Act:
		utils::StackLogger stopwatch("DeleteOneToOnePerformance", utils::LogLevel::Warning);
		auto results = context.bulkWriter().bulkDelete<AccountStates>("accounts", context.accountStates(), test::CreateFilter).get();

		// Assert:
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		EXPECT_EQ(0u, context.numEntitiesInCollection("accounts"));
		AssertResult(0, 0, 0, Num_Entities, 0, aggregate);
	}

	NO_STRESS_TEST(TEST_CLASS, DeleteOneToManyPerformance) {
		// Arrange:
		PerformanceContext context;
		context.bulkWriter().bulkUpsert<AccountStates>(
				"accounts",
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		EXPECT_EQ(Num_Entities, context.numEntitiesInCollection("accounts"));

		// Act: simulate a multi-delete by passing in a single element vector and a select all filter
		//      the createFilter function is called once and returns a filter that matches all documents
		utils::StackLogger stopwatch("DeleteOneToManyPerformance", utils::LogLevel::Warning);
		auto createSelectAllFilter = [](auto) { return document{} << finalize; };
		auto results = context.bulkWriter().bulkDelete<std::vector<int>>("accounts", { 1 }, createSelectAllFilter).get();

		// Assert:
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		EXPECT_EQ(0u, context.numEntitiesInCollection("accounts"));
		AssertResult(0, 0, 0, Num_Entities, 0, aggregate);
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
				auto pRegistry = test::CreateDefaultMongoTransactionRegistry();
				auto createDocument = [&blockFlag, &capture, pRegistry](const auto& txElement, auto index) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateDocumentCalls;
					capture.pCreateDocumentElement = &txElement;
					return CreateDocument(txElement, Height(1), index, *pRegistry);
				};

				// Act:
				return writer.bulkInsert<TransactionElements>("transactions", elements, createDocument);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkInsert<TransactionElements>("transactions", {}, CreateDocumentThrow<model::TransactionElement>);
			}

			static void AssertDelegation(const TransactionElements& elements, const Capture& capture, const BulkWriteResult& aggregate) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateDocumentCalls);
				EXPECT_EQ(&(*elements.cbegin()), capture.pCreateDocumentElement);
				AssertResult(1, 0, 0, 0, 0, aggregate);
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
				auto pRegistry = test::CreateDefaultMongoTransactionRegistry();
				auto createDocuments = [&blockFlag, &capture, pRegistry](const auto& txElement, auto index) {
					WAIT_FOR_EXPR(!blockFlag);
					++capture.NumCreateDocumentsCalls;
					capture.pCreateDocumentsElement = &txElement;

					// add three documents per entity
					return CreateDocuments(txElement, Height(1), index, *pRegistry);
				};

				// Act:
				return writer.bulkInsert<TransactionElements>("transactions", elements, createDocuments);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkInsert<TransactionElements>("transactions", {}, CreateDocumentsThrow<model::TransactionElement>);
			}

			static void AssertDelegation(const TransactionElements& elements, const Capture& capture, const BulkWriteResult& aggregate) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateDocumentsCalls);
				EXPECT_EQ(&(*elements.cbegin()), capture.pCreateDocumentsElement);
				AssertResult(3, 0, 0, 0, 0, aggregate); // 3 documents should have been inserted
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
				return writer.bulkUpsert<AccountStates>("accounts", accountStates, createDocument, createFilter);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkUpsert<AccountStates>(
						"accounts",
						{},
						CreateDocumentThrow<AccountStates::value_type>,
						CreateFilterThrow<AccountStates::value_type>);
			}

			static void AssertDelegation(const AccountStates& accountStates, const Capture& capture, const BulkWriteResult& aggregate) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateDocumentCalls);
				EXPECT_EQ((*accountStates.cbegin()).get(), capture.pCreateDocumentAccountState);

				EXPECT_EQ(1u, capture.NumCreateFilterCalls);
				EXPECT_EQ((*accountStates.cbegin()).get(), capture.pCreateFilterAccountState);

				AssertResult(0, 0, 0, 0, 1, aggregate);
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
				return writer.bulkDelete<AccountStates>("accounts", accountStates, createFilter);
			}

			static auto ExecuteZero(MongoBulkWriter& writer) {
				// Act:
				return writer.bulkDelete<AccountStates>("accounts", {}, CreateFilterThrow<AccountStates::value_type>);
			}

			static void AssertDelegation(const AccountStates& accountStates, const Capture& capture, const BulkWriteResult& aggregate) {
				// Assert:
				EXPECT_EQ(1u, capture.NumCreateFilterCalls);
				EXPECT_EQ((*accountStates.cbegin()).get(), capture.pCreateFilterAccountState);

				// - note that nothing was deleted because the db is empty
				AssertResult(0, 0, 0, 0, 0, aggregate);
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
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		TTraits::AssertDelegation(TTraits::GetElements(context), capture, aggregate);
	}

	// endregion

	// region shutdown

	BULK_OPERATION_TEST(FutureIsFulfilledEvenIfWriterIsDestroyed) {
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
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(future.get()));

		// Assert: the write should have completed successfully
		TTraits::AssertDelegation(TTraits::GetElements(context), capture, aggregate);
	}

	// endregion

	// region zero entities

	BULK_OPERATION_TEST(BulkOperationCanProcessZeroEntities) {
		// Arrange:
		PerformanceContext context(1);

		// Act:
		auto results = TTraits::ExecuteZero(context.bulkWriter()).get();

		// Assert:
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		AssertResult(0, 0, 0, 0, 0, aggregate);
	}

	// endregion

	// region bulk writer exception

	TEST(TEST_CLASS, FutureExposesBulkWriteExceptions) {
		// Arrange:
		// - insert one account into the db, then attempt to insert the same account again
		// - this will result in a bulk writer exception due to a duplicate key (index on account address)
		PerformanceContext context(1);
		context.bulkWriter().bulkUpsert<AccountStates>(
				"accounts",
				context.accountStates(),
				CreateAccountDocument,
				test::CreateFilter).get();

		// Sanity:
		EXPECT_EQ(1u, context.numEntitiesInCollection("accounts"));

		// Act: note that the statement will not throw since we are only calling get() on the future that holds the bulk write futures
		auto results = context.bulkWriter().bulkInsert<AccountStates>(
				"accounts",
				context.accountStates(),
				CreateAccountDocument).get();

		// Assert: the array of futures holds an exceptional future
		EXPECT_THROW(thread::get_all(std::move(results)), catapult_runtime_error);
	}

	// endregion
}}}
