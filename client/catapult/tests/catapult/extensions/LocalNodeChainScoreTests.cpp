#include "catapult/extensions/LocalNodeChainScore.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS LocalNodeChainScoreTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultChainScore) {
		// Act:
		LocalNodeChainScore score;
		auto scoreArray = score.get().toArray();

		// Assert:
		EXPECT_EQ(0u, scoreArray[0]);
		EXPECT_EQ(0u, scoreArray[1]);
	}

	TEST(TEST_CLASS, CanCreateChainScoreFromModelChainScore) {
		// Act:
		LocalNodeChainScore score(model::ChainScore(0x8FDE42679C23D678, 0x7A6B3481023543B6));
		auto scoreArray = score.get().toArray();

		// Assert:
		EXPECT_EQ(0x8FDE42679C23D678, scoreArray[0]);
		EXPECT_EQ(0x7A6B3481023543B6, scoreArray[1]);
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddToChainScore) {
		// Arrange:
		LocalNodeChainScore score(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));

		// Act:
		const auto& result = score += model::ChainScore(0x1000'0002'F000'0010, 0x0200'0500'0030'0005);
		auto scoreArray = score.get().toArray();

		// Assert:
		EXPECT_EQ(&score, &result);
		EXPECT_EQ(0x9FDE'426A'8C23'D688, scoreArray[0]);
		EXPECT_EQ(0x7C6B'3981'0265'43BB, scoreArray[1]);
	}

	// endregion
}}
