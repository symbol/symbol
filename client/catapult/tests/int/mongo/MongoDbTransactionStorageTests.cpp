#include "plugins/mongo/coremongo/src/MongoDbTransactionStorage.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoTransactionPlugin.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "tests/test/cache/UnconfirmedTransactionsTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/test/mongo/mocks/MockTransactionMapper.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

using namespace bsoncxx::builder::stream;

#define TEST_CLASS MongoDbTransactionStorageTests

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		constexpr size_t Num_Transactions = 10;

		void PrepareDatabase(const std::string& dbName) {
			mongocxx::instance::current();
			auto connection = test::CreateDbConnection();

			connection[dbName].drop();
		}

		std::vector<model::TransactionInfo> CreateTransactionInfos(
				size_t count,
				const std::function<Timestamp (size_t)>& deadlineGenerator) {
			std::vector<model::TransactionInfo> infos;
			for (auto i = 0u; i < count; ++i) {
				auto pTransaction = mocks::CreateMockTransaction(10);
				pTransaction->Deadline = deadlineGenerator(i);
				auto info = model::TransactionInfo(
						std::move(pTransaction),
						test::GenerateRandomData<Hash256_Size>(),
						test::GenerateRandomData<Hash256_Size>());
				infos.push_back(std::move(info));
			}

			return infos;
		}
	}

	// region transaction storage

	namespace {
		auto DeadlineGenerator(size_t i) {
			return Timestamp(10 * (i + 1));
		}

		auto CreateStorage(std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin) {
			PrepareDatabase(test::DatabaseName());
			auto pWriter = MongoBulkWriter::Create(test::DefaultDbUri(), test::DatabaseName(), test::CreateStartedIoServiceThreadPool(8));
			auto pConfig = std::make_shared<MongoStorageConfiguration>(test::DefaultDbUri(), test::DatabaseName(), pWriter);

			auto pRegistry = std::make_shared<MongoTransactionRegistry>();
			pRegistry->registerPlugin(std::move(pTransactionPlugin));
			return std::make_unique<MongoDbTransactionStorage>(pConfig, pRegistry);
		}

		void AssertSuccessfulSaves(const std::vector<bool> successfulSaves) {
			auto i = 0u;
			for (auto isSaveSuccessful : successfulSaves)
				EXPECT_TRUE(isSaveSuccessful) << "save at index " << i++;
		}

		void AssertDependentDocuments(
				mongocxx::collection& utCollection,
				const Key& transactionSigner,
				size_t expectedNumDependentDocuments) {
			// sort by counter and filter by (aggregate) signer
			mongocxx::options::find options;
			options.sort(document{} << "dd_counter" << 1 << finalize);
			auto filter = document{} << "aggregate_signer" << mappers::ToBinary(transactionSigner) << finalize;
			ASSERT_EQ(expectedNumDependentDocuments, utCollection.count(filter.view()));

			auto cursor = utCollection.find(filter.view(), options);
			auto iter = cursor.begin();
			for (auto i = 0u; i < expectedNumDependentDocuments; ++i) {
				EXPECT_EQ(i, (*iter)["dd_counter"].get_int32().value) << "dependent document at " << i;
				++iter;
			}
		}

		void AssertTransactions(
				const std::vector<model::TransactionInfo>& expectedTransactionInfos,
				size_t expectedNumDependentDocuments = 0) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto utCollection = database["unconfirmedTransactions"];

			// MockTransactionMongoPlugin creates dependent documents with 'dd_counter' property, so documents without that property
			// should be transactions
			auto txFilter = document{} << "dd_counter" << open_document << "$exists" << false << close_document << finalize;

			// sort the retrieved transactions by deadline since the bulk writer doesn't
			// guarantee any ordering while inserting the documents
			mongocxx::options::find options;
			options.sort(document{} << "transaction.deadline" << 1 << finalize);

			// Assert: check collection size
			EXPECT_EQ(expectedTransactionInfos.size(), utCollection.count(txFilter.view()));
			EXPECT_EQ((1 + expectedNumDependentDocuments) * expectedTransactionInfos.size(), utCollection.count(document{} << finalize));

			auto txCursor = utCollection.find(txFilter.view(), options);
			auto iter = txCursor.begin();
			for (const auto& expectedTransactionInfo : expectedTransactionInfos) {
				const auto& expectedTransaction = reinterpret_cast<const mocks::MockTransaction&>(*expectedTransactionInfo.pEntity);
				const auto& transactionDocument = (*iter)["transaction"].get_document().value;
				test::AssertEqualMockTransactionData(expectedTransaction, transactionDocument);

				const auto& transactionMeta = (*iter)["meta"].get_document().value;
				auto expectedMetadata = MongoTransactionMetadata(
						expectedTransactionInfo.EntityHash,
						expectedTransactionInfo.MerkleComponentHash);
				test::AssertEqualTransactionMetadata(expectedMetadata, transactionMeta);

				// - check dependent documents
				AssertDependentDocuments(utCollection, expectedTransactionInfo.pEntity->Signer, expectedNumDependentDocuments);
				++iter;
			}
		}

		void AssertUtCount(int64_t count) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto filter = document{} << finalize;
			EXPECT_EQ(count, database["unconfirmedTransactions"].count(filter.view()));
		}

		class TransactionStorageContext {
		public:
			explicit TransactionStorageContext(size_t numTransactionInfos)
					: TransactionStorageContext(numTransactionInfos, mocks::CreateMockTransactionMongoPlugin())
			{}

			TransactionStorageContext(size_t numTransactionInfos, std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin)
					: m_pStorage(CreateStorage(std::move(pTransactionPlugin)))
					, m_transactionInfos(CreateTransactionInfos(numTransactionInfos, DeadlineGenerator))
			{}

		public:
			MongoDbTransactionStorage& storage() {
				return *m_pStorage;
			}

			std::vector<model::TransactionInfo>& transactionInfos() {
				return m_transactionInfos;
			}

			bool saveTransaction(const model::TransactionInfo& transactionInfo) {
				auto isSuccessful = m_pStorage->saveTransaction(transactionInfo);
				m_pStorage->commit();
				return isSuccessful;
			}

			void removeTransaction(const Hash256& hash) {
				m_pStorage->removeTransaction(hash);
				m_pStorage->commit();
			}

			void removeTransactions(const std::vector<model::TransactionInfo>& transactionInfos) {
				m_pStorage->removeTransactions(transactionInfos);
				m_pStorage->commit();
			}

			void pruneTransactions(Timestamp timestamp) {
				m_pStorage->pruneTransactions(timestamp);
			}

			void seedDatabase() {
				std::vector<bool> successfulSaves;
				for (const auto& transactionInfo : m_transactionInfos)
					successfulSaves.push_back(saveTransaction(transactionInfo));

				// Sanity:
				AssertSuccessfulSaves(successfulSaves);
			}

		private:
			std::unique_ptr<MongoDbTransactionStorage> m_pStorage;
			std::vector<model::TransactionInfo> m_transactionInfos;
		};
	}

	// region saveTransaction

	TEST(TEST_CLASS, CanSaveSingleTransaction) {
		// Arrange:
		TransactionStorageContext context(1);

		// Act:
		auto isSaveSuccessful = context.saveTransaction(context.transactionInfos().back());

		// Assert:
		EXPECT_TRUE(isSaveSuccessful);
		AssertTransactions(context.transactionInfos());
	}

	TEST(TEST_CLASS, CanSaveSingleTransactionWithDependentDocuments) {
		// Arrange:
		TransactionStorageContext context(1, mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, 3));

		// Act:
		auto isSaveSuccessful = context.saveTransaction(context.transactionInfos().back());

		// Assert:
		EXPECT_TRUE(isSaveSuccessful);
		AssertTransactions(context.transactionInfos(), 3);
	}

	TEST(TEST_CLASS, CanSaveMultipleTransactions) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		std::vector<bool> successfulSaves;

		// Act:
		for (const auto& transactionInfo : context.transactionInfos())
			successfulSaves.push_back(context.saveTransaction(transactionInfo));

		// Assert:
		AssertSuccessfulSaves(successfulSaves);
		AssertTransactions(context.transactionInfos());
	}

	// endregion

	// region removeTransaction(s)

	TEST(TEST_CLASS, CanRemoveSingleTransaction) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		// Act:
		context.removeTransaction(context.transactionInfos()[4].EntityHash);
		context.transactionInfos().erase(context.transactionInfos().cbegin() + 4);

		// Assert:
		AssertUtCount(Num_Transactions - 1);
		AssertTransactions(context.transactionInfos());
	}

	TEST(TEST_CLASS, CanRemoveSingleTransactionWithDependentDocuments) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		// - add dependent documents
		{
			auto connection = test::CreateDbConnection();
			auto collection = connection[test::DatabaseName()]["unconfirmedTransactions"];

			auto filter = document{} << "meta.hash" << mappers::ToBinary(context.transactionInfos()[4].EntityHash) << finalize;
			auto aggregateTransactionDocument = collection.find_one(filter.view());
			auto aggregateId = aggregateTransactionDocument->view()["_id"].get_oid().value;

			for (auto i = 0; i < 3; ++i) {
				auto dependentDocument = document{}
						<< "meta" << open_document << "aggregateId" << aggregateId << close_document
						<< "transaction" << open_document << "i" << i << close_document
						<< finalize;

				collection.insert_one(dependentDocument.view()).get();
			}
		}

		// Sanity: the dependent documents were added to the storage
		AssertUtCount(Num_Transactions + 3);

		// Act:
		context.removeTransaction(context.transactionInfos()[4].EntityHash);
		context.transactionInfos().erase(context.transactionInfos().cbegin() + 4);

		// Assert: all dependent documents were also deleted
		AssertUtCount(Num_Transactions - 1);
		AssertTransactions(context.transactionInfos());
	}

	TEST(TEST_CLASS, CanRemoveMultipleTransactions) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		// Act: remove all entries which had an even index
		for (auto i = 0u; i < Num_Transactions / 2; ++i) {
			context.removeTransaction(context.transactionInfos()[i].EntityHash);
			context.transactionInfos().erase(context.transactionInfos().cbegin() + i);
		}

		// Assert:
		AssertUtCount(Num_Transactions / 2);
		AssertTransactions(context.transactionInfos());
	}

	TEST(TEST_CLASS, CanRemoveTransactions) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		// Act:
		context.removeTransactions(context.transactionInfos());

		// Assert:
		AssertTransactions({});
	}

	TEST(TEST_CLASS, CanRemoveMultipleTransactionsWithSingleCall) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		std::vector<model::TransactionInfo> infosToRemove;
		for (auto i = 0u; i < Num_Transactions / 2; ++i) {
			const auto& info = context.transactionInfos()[i];
			infosToRemove.emplace_back(info.copy());
			context.transactionInfos().erase(context.transactionInfos().cbegin() + i);
		}

		// Act: remove all entries with a single call
		context.removeTransactions(infosToRemove);

		// Assert:
		AssertUtCount(Num_Transactions / 2);
		AssertTransactions(context.transactionInfos());
	}

	// endregion

	// region saveTransaction + removeTransaction

	TEST(TEST_CLASS, CanRemoveAndSaveSameTransactions) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		// - remove all entries with even index
		for (auto i = 0u; i < Num_Transactions / 2; ++i)
			context.storage().removeTransaction(context.transactionInfos()[2 * i].EntityHash);

		// Sanity: the transactions were not removed from the storage but only added to the deleted hashes set
		AssertUtCount(Num_Transactions);

		// Act: save transactions that have been removed
		for (auto i = 0u; i < Num_Transactions / 2; ++i)
			context.storage().saveTransaction(context.transactionInfos()[2 * i]);

		context.storage().commit();

		// Assert: all transactions remain because all removed ones were (re)saved
		AssertTransactions(context.transactionInfos());
	}

	TEST(TEST_CLASS, CanSaveAndRemoveSameTransactions) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);

		// - save all entries
		for (const auto& transactionInfo : context.transactionInfos())
			context.storage().saveTransaction(transactionInfo);

		// Sanity: the transactions were not committed to the storage yet
		AssertUtCount(0);

		// Act: remove all entries with even index
		for (auto i = 0u; i < Num_Transactions / 2; ++i)
			context.storage().removeTransaction(context.transactionInfos()[2 * i].EntityHash);

		context.storage().commit();

		// Assert: only odd entries remain
		std::vector<model::TransactionInfo> expectedTransactionInfos;
		for (auto i = 0u; i < Num_Transactions / 2; ++i)
			expectedTransactionInfos.push_back(std::move(context.transactionInfos()[2 * i + 1]));

		AssertUtCount(Num_Transactions / 2);
		AssertTransactions(expectedTransactionInfos);
	}

	// endregion

	// region pruneTransactions

	namespace {
		void AssertPruning(size_t pruneCount, Timestamp pruneTime, const std::vector<size_t> expectedIndexes) {
			// Arrange: transactions will have timestamps in ascending order beginning at 10 and increasing by 10
			TransactionStorageContext context(Num_Transactions);
			context.seedDatabase();

			// Act:
			for (auto i = 0u; i < pruneCount; ++i)
				context.pruneTransactions(pruneTime);

			// Assert:
			std::vector<model::TransactionInfo> expectedTransactionInfos;
			for (auto index : expectedIndexes)
				expectedTransactionInfos.push_back(std::move(context.transactionInfos()[index]));

			AssertTransactions(expectedTransactionInfos);
		}
	}

	TEST(TEST_CLASS, CanPruneTransactions) {
		// Assert: transaction at index 6 has deadline 70
		AssertPruning(1, Timestamp(70), { 6, 7, 8, 9 });
	}

	TEST(TEST_CLASS, PruneIsNoOpIfAllDeadlinesAreEqualOrLargerThanPruneTime) {
		// Assert: all transactions have deadlines >= 10
		AssertPruning(1, Timestamp(10), { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
	}

	TEST(TEST_CLASS, PruneIsIdempotent) {
		// Assert:
		AssertPruning(10, Timestamp(50), { 4, 5, 6, 7, 8, 9 });
	}

	// endregion

	// region commit

	namespace {
		void RemoveEverySecondTransactionInfo(
				std::vector<model::TransactionInfo>& transactionInfos,
				MongoDbTransactionStorage& storage) {
			auto i = 0u;
			for (auto iter = transactionInfos.begin(); transactionInfos.end() != iter;) {
				if (1 == ++i % 2) {
					storage.removeTransaction(iter->EntityHash);
					iter = transactionInfos.erase(iter);
					continue;
				}

				++iter;
			}
		}
	}

	TEST(TEST_CLASS, CommitExecutesSaveBatches) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		auto& transactionInfos = context.transactionInfos();
		for (const auto& transactionInfo : transactionInfos)
			context.storage().saveTransaction(transactionInfo);

		// Sanity:
		AssertUtCount(0);

		// Act:
		context.storage().commit();

		// Assert:
		AssertUtCount(Num_Transactions);
		AssertTransactions(transactionInfos);
	}

	TEST(TEST_CLASS, CommitExecutesRemoveBatches) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();

		// Sanity:
		AssertUtCount(Num_Transactions);

		// remove every second transaction
		RemoveEverySecondTransactionInfo(context.transactionInfos(), context.storage());

		// Sanity:
		AssertUtCount(Num_Transactions);

		// Act:
		context.storage().commit();

		// Assert:
		AssertUtCount(Num_Transactions / 2);
		AssertTransactions(context.transactionInfos());
	}

	TEST(TEST_CLASS, CommitExecutesAllBatchTypes) {
		// Arrange:
		TransactionStorageContext context(Num_Transactions);
		context.seedDatabase();
		auto additonalTransactionInfos = CreateTransactionInfos(3, [](size_t i) { return Timestamp(1234 + i); });

		// Sanity:
		AssertUtCount(Num_Transactions);

		// Act:
		// - remove every second transaction (ids 20, 40, 60, 80, 100 left)
		auto& transactionInfos = context.transactionInfos();
		RemoveEverySecondTransactionInfo(transactionInfos, context.storage());

		// - add a few transactions
		for (auto& transactionInfo : additonalTransactionInfos) {
			context.storage().saveTransaction(transactionInfo);
			transactionInfos.push_back(std::move(transactionInfo));
		}

		context.storage().commit();

		// Assert:
		AssertUtCount(5 + 3);
		AssertTransactions(transactionInfos);
	}

	// endregion
}}}
