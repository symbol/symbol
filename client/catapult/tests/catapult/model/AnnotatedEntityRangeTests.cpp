#include "catapult/model/AnnotatedEntityRange.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AnnotatedEntityRangeTests

	TEST(TEST_CLASS, CanCreateDefaultRange) {
		// Act:
		auto annotatedRange = AnnotatedEntityRange<Block>();

		// Assert:
		EXPECT_TRUE(annotatedRange.Range.empty());
		EXPECT_EQ(Key(), annotatedRange.SourcePublicKey);
	}

	TEST(TEST_CLASS, CanCreateAroundRange) {
		// Arrange:
		auto range = test::CreateBlockEntityRange(3);
		const auto* pRangeData = range.data();

		// Act:
		auto annotatedRange = AnnotatedEntityRange<Block>(std::move(range));

		// Assert:
		EXPECT_EQ(pRangeData, annotatedRange.Range.data());
		EXPECT_EQ(Key(), annotatedRange.SourcePublicKey);
	}

	TEST(TEST_CLASS, CanCreateAroundRangeAndContext) {
		// Arrange:
		auto sourcePublicKey = test::GenerateRandomData<Key_Size>();
		auto range = test::CreateBlockEntityRange(3);
		const auto* pRangeData = range.data();

		// Act:
		auto annotatedRange = AnnotatedEntityRange<Block>(std::move(range), sourcePublicKey);

		// Assert:
		EXPECT_EQ(pRangeData, annotatedRange.Range.data());
		EXPECT_EQ(sourcePublicKey, annotatedRange.SourcePublicKey);
	}
}}
