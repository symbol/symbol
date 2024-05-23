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

		// region test utils

		using TransactionStatusesMap = std::unordered_map<Hash256, model::TransactionStatus, utils::ArrayHasher<Hash256>>;

		auto PrepareDatabaseAndCreateMongoContext(thread::IoThreadPool& pool) {
			test::PrepareDatabase(test::DatabaseName());
			return test::CreateDefaultMongoStorageContext(test::DatabaseName(), pool);
		}

		auto CreateTransactionStatuses(size_t count) {
			std::vector<model::TransactionStatus> statuses;
			for (auto i = 1u; i <= count; ++i)
				statuses.emplace_back(test::GenerateRandomByteArray<Hash256>(), Timestamp(i * i), i);

			return statuses;
		}

		void AssertTransactionStatuses(const TransactionStatusesMap& expectedTransactionStatuses) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto collection = database[Collection_Name];

			// Assert: check collection size
			EXPECT_EQ(expectedTransactionStatuses.size(), static_cast<size_t>(collection.count_documents({})));

			auto txCursor = collection.find({});
			for (const auto& view : txCursor) {
				auto statusView = view["status"].get_document().view();

				auto dbHash = test::GetHashValue(statusView, "hash");
				auto expectedIter = expectedTransactionStatuses.find(dbHash);
				ASSERT_TRUE(expectedTransactionStatuses.cend() != expectedIter);

				const auto& status = expectedIter->second;
				EXPECT_EQ(status.Hash, dbHash);
				EXPECT_EQ(status.Status, test::GetUint32(statusView, "code"));
				EXPECT_EQ(status.Deadline, Timestamp(test::GetUint64(statusView, "deadline")));
			}
		}

		// endregion

		// region TransactionStatusSubscriberContext

		class TransactionStatusSubscriberContext {
		public:
			explicit TransactionStatusSubscriberContext(size_t numTransactionStatuses)
					: m_pPool(test::CreateStartedIoThreadPool(test::Num_Default_Mongo_Test_Pool_Threads))
					, m_pMongoContext(PrepareDatabaseAndCreateMongoContext(*m_pPool))
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

			TransactionStatusesMap toMap(size_t startIndex = 0) const {
				TransactionStatusesMap map;

				auto i = 0u;
				for (const auto& status : m_statuses) {
					if (startIndex > i++)
						continue;

					map.emplace(status.Hash, status);
				}

				return map;
			}

		public:
			void saveTransactionStatus(const model::TransactionStatus& status) {
				m_pSubscriber->notifyStatus(*test::GenerateTransactionWithDeadline(status.Deadline), status.Hash, status.Status);
				m_pSubscriber->flush();
			}

		private:
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::unique_ptr<MongoStorageContext> m_pMongoContext;
			std::unique_ptr<subscribers::TransactionStatusSubscriber> m_pSubscriber;
			std::vector<model::TransactionStatus> m_statuses;
		};

		// endregion
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

	TEST(TEST_CLASS, TransactionStatusCollectionIsCapped) {
		// Arrange: max is set to 25 in PrepareDatabase
		TransactionStatusSubscriberContext context(30);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		for (const auto& status : context.statuses())
			context.saveTransactionStatus(status);

		// Assert: only the 25 most recent statuses are saved
		AssertTransactionStatuses(context.toMap(5));
	}

	// endregion

	// region flush

	TEST(TEST_CLASS, FlushWritesPendingBufferedData) {
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

	TEST(TEST_CLASS, FlushIsNoOpWhenNoDataIsPending) {
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
