#include "mongo/src/MongoTransactionStatusStorage.h"
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/TransactionStatus.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>
#include <unordered_set>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoTransactionStatusStorageTests

	namespace {
		constexpr size_t Num_Transaction_Statuses = 10;
		constexpr auto Collection_Name = "transactionStatuses";

		using TransactionStatusesMap = std::unordered_map<Hash256, model::TransactionStatus, utils::ArrayHasher<Hash256>>;

		auto ResetDatabaseAndCreateMongoContext() {
			test::ResetDatabase(test::DatabaseName());
			return test::CreateDefaultMongoStorageContext(test::DatabaseName());
		}

		auto CreateTransactionStatuses(size_t count) {
			std::vector<model::TransactionStatus> statuses;
			for (auto i = 0u; i < count; ++i)
				statuses.emplace_back(test::GenerateRandomData<Hash256_Size>(), i, Timestamp(i * i));

			return statuses;
		}

		class TransactionStatusSubscriberContext {
		public:
			explicit TransactionStatusSubscriberContext(size_t numTransactionStatuses)
					: m_pMongoContext(ResetDatabaseAndCreateMongoContext())
					, m_pSubscriber(CreateMongoTransactionStatusStorage(*m_pMongoContext))
					, m_statuses(CreateTransactionStatuses(numTransactionStatuses)) {
			}

		public:
			subscribers::TransactionStatusSubscriber& subscriber() {
				return *m_pSubscriber;
			}

			const std::vector<model::TransactionStatus>& statuses() const {
				return m_statuses;
			}

			TransactionStatusesMap toMap() {
				TransactionStatusesMap map;
				for (const auto& status : m_statuses)
					map.emplace(status.Hash, status);

				return map;
			}

			void saveTransactionStatus(const model::TransactionStatus& status) {
				m_pSubscriber->notifyStatus(*test::GenerateTransactionWithDeadline(status.Deadline), status.Hash, status.Status);
				m_pSubscriber->flush();
			}

		private:
			std::unique_ptr<MongoStorageContext> m_pMongoContext;
			std::unique_ptr<subscribers::TransactionStatusSubscriber> m_pSubscriber;
			std::vector<model::TransactionStatus> m_statuses;
		};

		void AssertTransactionStatuses(const TransactionStatusesMap& expectedTransactionStatuses) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto collection = database[Collection_Name];

			// Assert: check collection size
			EXPECT_EQ(expectedTransactionStatuses.size(), collection.count({}));

			auto txCursor = collection.find({});
			for (const auto& view : txCursor) {
				Hash256 dbHash;
				mappers::DbBinaryToModelArray(dbHash, view["hash"].get_binary());
				auto expectedIter = expectedTransactionStatuses.find(dbHash);
				ASSERT_TRUE(expectedTransactionStatuses.cend() != expectedIter);

				const auto& status = expectedIter->second;
				EXPECT_EQ(status.Hash, dbHash);
				EXPECT_EQ(status.Status, test::GetUint32(view, "status"));
				EXPECT_EQ(status.Deadline.unwrap(), test::GetUint64(view, "deadline"));
			}
		}
	}

	// region notifyStatus

	TEST(TEST_CLASS, CanSaveSingleTransactionStatus) {
		// Arrange:
		TransactionStatusSubscriberContext context(1);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		context.saveTransactionStatus(context.statuses().back());

		// Assert:
		AssertTransactionStatuses(context.toMap());
	}

	TEST(TEST_CLASS, CanSaveMultipleTransactionStatuses) {
		// Arrange:
		TransactionStatusSubscriberContext context(Num_Transaction_Statuses);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		for (const auto& status : context.statuses())
			context.saveTransactionStatus(status);

		// Assert:
		AssertTransactionStatuses(context.toMap());
	}

	TEST(TEST_CLASS, CanSaveMultipleTransactionStatuses_SameTransaction) {
		// Arrange:
		TransactionStatusSubscriberContext context(0);
		auto statuses = CreateTransactionStatuses(1);
		auto& status = statuses.back();
		context.saveTransactionStatus(status);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 1);

		// Act:
		for (auto i = 0u; i < 10; ++i) {
			status.Status = i;
			context.saveTransactionStatus(status);
		}

		// Assert: last status that was saved should be in the db
		TransactionStatusesMap map;
		map.emplace(status.Hash, status);
		AssertTransactionStatuses(map);
	}

	// endregion

	// region flush

	TEST(TEST_CLASS, FlushExecutesSaveBatches) {
		// Arrange:
		TransactionStatusSubscriberContext context(Num_Transaction_Statuses);
		auto& transactionStatuses = context.statuses();
		for (const auto& status : transactionStatuses)
			context.subscriber().notifyStatus(*test::GenerateTransactionWithDeadline(status.Deadline), status.Hash, status.Status);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		context.subscriber().flush();

		// Assert:
		test::AssertCollectionSize(Collection_Name, transactionStatuses.size());
	}

	TEST(TEST_CLASS, FlushIsNoOpIfNoDataIsPending) {
		// Arrange:
		TransactionStatusSubscriberContext context(Num_Transaction_Statuses);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		context.subscriber().flush();

		// Assert:
		test::AssertCollectionSize(Collection_Name, 0);
	}

	// endregion
}}
