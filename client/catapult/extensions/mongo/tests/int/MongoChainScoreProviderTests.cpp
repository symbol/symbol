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

#include "mongo/src/MongoChainScoreProvider.h"
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoChainStatisticUtils.h"
#include "catapult/model/ChainScore.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoChainScoreProviderTests

	namespace {
		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(MongoErrorPolicy::Mode errorPolicyMode = MongoErrorPolicy::Mode::Strict)
					: m_pPool(test::CreateStartedIoThreadPool(test::Num_Default_Mongo_Test_Pool_Threads))
					, m_mongoContext(
							test::DefaultDbUri(),
							test::DatabaseName(),
							MongoBulkWriter::Create(test::DefaultDbUri(), test::DatabaseName(), *m_pPool),
							errorPolicyMode)
					, m_pScoreProvider(CreateMongoChainScoreProvider(m_mongoContext))
			{}

		public:
			ChainScoreProvider& chainScoreProvider() {
				return *m_pScoreProvider;
			}

		private:
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			MongoStorageContext m_mongoContext;
			std::unique_ptr<ChainScoreProvider> m_pScoreProvider;
		};
	}

	namespace {
		void AssertDbScore(const model::ChainScore& expectedScore) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			auto cursor = database["chainStatistic"].find({});
			ASSERT_EQ(1, std::distance(cursor.begin(), cursor.end()));

			auto matchedDocument = database["chainStatistic"].find_one({}).value();
			auto currentView = matchedDocument.view()["current"].get_document().view();

			auto scoreLow = test::GetUint64(currentView, "scoreLow");
			auto scoreHigh = test::GetUint64(currentView, "scoreHigh");
			EXPECT_EQ(expectedScore, model::ChainScore(scoreHigh, scoreLow));
		}
	}

	TEST(TEST_CLASS, CanSaveScore) {
		// Arrange:
		TestContext context;
		model::ChainScore score(0x12345670, 0x89ABCDEF);

		// Act:
		context.chainScoreProvider().saveScore(score);

		// Assert:
		AssertDbScore(score);
	}

	TEST(TEST_CLASS, CannotSaveSameScoreTwiceWhenErrorModeIsStrict) {
		// Arrange:
		TestContext context;
		model::ChainScore score(0x12345670, 0x89ABCDEF);

		context.chainScoreProvider().saveScore(score);

		// Act + Assert:
		EXPECT_THROW(context.chainScoreProvider().saveScore(score), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSaveSameScoreTwiceWhenErrorModeIsIdempotent) {
		// Arrange:
		TestContext context(MongoErrorPolicy::Mode::Idempotent);
		model::ChainScore score(0x12345670, 0x89ABCDEF);

		context.chainScoreProvider().saveScore(score);

		// Act:
		context.chainScoreProvider().saveScore(score);

		// Assert:
		AssertDbScore(score);
	}

	TEST(TEST_CLASS, SaveScoreDoesNotOverwriteHeight) {
		// Arrange:
		TestContext context;
		model::ChainScore score(0x12345670, 0x89ABCDEF);

		// - set the height
		auto connection = test::CreateDbConnection();
		auto database = connection[test::DatabaseName()];
		auto heightDocument = document()
				<< "$set" << open_document
					<< "current.height" << static_cast<int64_t>(123)
				<< close_document
				<< finalize;
		TrySetChainStatisticDocument(database, heightDocument.view());

		// Act:
		context.chainScoreProvider().saveScore(score);

		// Assert: the height is unchanged
		auto chainStatisticDocument = GetChainStatisticDocument(database);
		auto currentView = chainStatisticDocument.view()["current"].get_document().view();
		EXPECT_EQ(3u, test::GetFieldCount(currentView));
		EXPECT_EQ(123u, test::GetUint64(currentView, "height"));
	}
}}
