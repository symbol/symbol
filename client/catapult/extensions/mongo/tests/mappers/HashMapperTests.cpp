#include "mongo/src/mappers/HashMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS HashMapperTests

	namespace {
		auto CreateDbHash(const Hash256& hash) {
			return bson_stream::document() << "meta"
					<< bson_stream::open_document
						<< "hash" << ToBinary(hash)
					<< bson_stream::close_document
					<< bson_stream::finalize;
		}

		auto CreateViews(const std::vector<bsoncxx::document::value>& values) {
			std::vector<bsoncxx::document::view> views;
			for (const auto& value : values)
				views.push_back(value.view());
			return views;
		}

		auto GenerateValues(const model::HashRange& hashRange) {
			std::vector<bsoncxx::document::value> values;
			for (const auto& hash : hashRange)
				values.push_back(CreateDbHash(hash));

			return values;
		}
	}

	TEST(TEST_CLASS, CanMapDbHashes) {
		// Arrange:
		auto expectedRange = model::HashRange::CopyFixed(test::GenerateRandomData<3 * Hash256_Size>().data(), 3);
		auto values = GenerateValues(expectedRange);
		auto views = CreateViews(values);

		// Act:
		auto range = ToModel(views, 3);

		// Assert:
		auto i = 0u;
		auto iter = range.cbegin();
		ASSERT_EQ(expectedRange.size(), range.size());
		for (const auto& hash : expectedRange) {
			EXPECT_EQ(hash, *iter) << "hash at " << i++;
			++iter;
		}
	}

	TEST(TEST_CLASS, RequestMoreDbHashesThanAvailableThrows) {
		auto values = GenerateValues(test::GenerateRandomHashes(10));
		auto views = CreateViews(values);

		// Act + Assert:
		EXPECT_THROW(ToModel(views, 11), catapult_runtime_error);
	}

	TEST(TEST_CLASS, RequestLessHashesThanAvailableThrows) {
		auto values = GenerateValues(test::GenerateRandomHashes(10));
		auto views = CreateViews(values);

		// Act + Assert:
		EXPECT_THROW(ToModel(views, 9), catapult_runtime_error);
	}
}}}
