#include "mongo/src/MongoChainInfoUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoChainInfoUtilsTests

	namespace {
		template<typename TAction>
		void RunTestWithDatabase(TAction action) {
			// Arrange:
			test::PrepareDatabase(test::DatabaseName());
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			// Act + Assert:
			action(database);
		}

		auto CreateUpdateDocument(int64_t value) {
			return document() << "$set" << open_document << "value" << value << close_document << finalize;
		}
	}

	TEST(TEST_CLASS, ChainInfoDocumentIsEmptyWhenNotSet) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			// Act:
			auto doc = GetChainInfoDocument(database);

			// Assert:
			EXPECT_TRUE(mappers::IsEmptyDocument(doc));
		});
	}

	TEST(TEST_CLASS, CanSetChainInfoDocument) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			// Act:
			SetChainInfoDocument(database, CreateUpdateDocument(12).view());
			auto doc = GetChainInfoDocument(database);

			// Assert:
			EXPECT_FALSE(mappers::IsEmptyDocument(doc));
			EXPECT_EQ(12u, test::GetUint64(doc.view(), "value"));
		});
	}

	TEST(TEST_CLASS, CanUpdateChainInfoDocument) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			SetChainInfoDocument(database, CreateUpdateDocument(12).view());

			// Act:
			SetChainInfoDocument(database, CreateUpdateDocument(15).view());
			auto doc = GetChainInfoDocument(database);

			// Assert:
			EXPECT_FALSE(mappers::IsEmptyDocument(doc));
			EXPECT_EQ(15u, test::GetUint64(doc.view(), "value"));
		});
	}
}}
