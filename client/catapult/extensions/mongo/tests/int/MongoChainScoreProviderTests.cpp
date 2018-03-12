#include "mongo/src/MongoChainScoreProvider.h"
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoChainInfoUtils.h"
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
			TestContext()
					: m_mongoContext(
							test::DefaultDbUri(),
							test::DatabaseName(),
							MongoBulkWriter::Create(test::DefaultDbUri(), test::DatabaseName(), test::CreateStartedIoServiceThreadPool(8)))
					, m_pScoreProvider(CreateMongoChainScoreProvider(m_mongoContext))
			{}

		public:
			ChainScoreProvider& chainScoreProvider() {
				return *m_pScoreProvider;
			}

		private:
			MongoStorageContext m_mongoContext;
			std::unique_ptr<ChainScoreProvider> m_pScoreProvider;
		};
	}

	namespace {
		void AssertDbScore(const model::ChainScore& expectedScore) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			auto cursor = database["chainInfo"].find({});
			ASSERT_EQ(1u, std::distance(cursor.begin(), cursor.end()));

			auto matchedDocument = database["chainInfo"].find_one({}).get();

			auto scoreLow = static_cast<uint64_t>(matchedDocument.view()["scoreLow"].get_int64().value);
			auto scoreHigh = static_cast<uint64_t>(matchedDocument.view()["scoreHigh"].get_int64().value);
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

	TEST(TEST_CLASS, CanLoadScore) {
		// Arrange:
		TestContext context;
		model::ChainScore score(0x12345670, 0x89ABCDEF);
		context.chainScoreProvider().saveScore(score);

		// Act:
		auto result = context.chainScoreProvider().loadScore();

		// Assert:
		EXPECT_EQ(score, result);
	}

	TEST(TEST_CLASS, SaveScoreDoesNotOverwriteHeight) {
		// Arrange:
		TestContext context;
		model::ChainScore score(0x12345670, 0x89ABCDEF);

		// - set the height
		auto connection = test::CreateDbConnection();
		auto database = connection[test::DatabaseName()];
		auto heightDocument = document() << "$set" << open_document << "height" << static_cast<int64_t>(123) << close_document << finalize;
		SetChainInfoDocument(database, heightDocument.view());

		// Act:
		context.chainScoreProvider().saveScore(score);

		// Assert: the height is unchanged
		auto chainInfoDocument = GetChainInfoDocument(database);
		EXPECT_EQ(4u, test::GetFieldCount(chainInfoDocument.view()));
		EXPECT_EQ(123u, test::GetUint64(chainInfoDocument.view(), "height"));
	}
}}
