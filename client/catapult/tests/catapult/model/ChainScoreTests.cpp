#include "catapult/model/ChainScore.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	// region constructor / assign

	TEST(ChainScoreTests, CanCreateDefaultChainScore) {
		// Act:
		ChainScore score;
		auto scoreArray = score.toArray();

		// Assert:
		EXPECT_EQ(0u, scoreArray[0]);
		EXPECT_EQ(0u, scoreArray[1]);
	}

	TEST(ChainScoreTests, CanCreateChainScoreFrom64BitValue) {
		// Act:
		ChainScore score(0x7A6B3481023543B6);
		auto scoreArray = score.toArray();

		// Assert:
		EXPECT_EQ(0u, scoreArray[0]);
		EXPECT_EQ(0x7A6B3481023543B6, scoreArray[1]);
	}

	TEST(ChainScoreTests, CanCreateChainScoreFrom128BitValue) {
		// Act:
		ChainScore score(0x8FDE42679C23D678, 0x7A6B3481023543B6);
		auto scoreArray = score.toArray();

		// Assert:
		EXPECT_EQ(0x8FDE42679C23D678, scoreArray[0]);
		EXPECT_EQ(0x7A6B3481023543B6, scoreArray[1]);
	}

	TEST(ChainScoreTests, CanCopyConstructChainScore) {
		// Act:
		ChainScore score(0x8FDE42679C23D678, 0x7A6B3481023543B6);
		ChainScore scoreCopy(score);
		auto scoreArray = scoreCopy.toArray();

		// Assert:
		EXPECT_EQ(0x8FDE42679C23D678, scoreArray[0]);
		EXPECT_EQ(0x7A6B3481023543B6, scoreArray[1]);
	}

	TEST(ChainScoreTests, CanAssignChainScore) {
		// Act:
		ChainScore score(0x8FDE42679C23D678, 0x7A6B3481023543B6);
		ChainScore scoreCopy;
		const auto& result = scoreCopy = score;
		auto scoreArray = scoreCopy.toArray();

		// Assert:
		EXPECT_EQ(&scoreCopy, &result);
		EXPECT_EQ(0x8FDE42679C23D678, scoreArray[0]);
		EXPECT_EQ(0x7A6B3481023543B6, scoreArray[1]);
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<ChainScore> GenerateIncreasingValues() {
			return {
				ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6),
				ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B7),
				ChainScore(0x8FDE'4267'9C23'D678, 0x8A6B'3481'0235'43B7),
				ChainScore(0x8FDE'4267'9C23'D679, 0x8A6B'3481'0235'43B7),
				ChainScore(0x9FDE'4267'9C23'D679, 0x8A6B'3481'0235'43B7)
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(ChainScoreTests, GenerateIncreasingValues())

	// endregion

	// region add / subtract

	TEST(ChainScoreTests, CanAddToChainScore) {
		// Arrange:
		ChainScore score(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6);

		// Act:
		const auto& result = score += ChainScore(0x1000'0002'F000'0010, 0x0200'0500'0030'0005);
		auto scoreArray = score.toArray();

		// Assert:
		EXPECT_EQ(&score, &result);
		EXPECT_EQ(0x9FDE'426A'8C23'D688, scoreArray[0]);
		EXPECT_EQ(0x7C6B'3981'0265'43BB, scoreArray[1]);
	}

	TEST(ChainScoreTests, CanSubtractFromChainScore) {
		// Arrange:
		ChainScore score(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6);

		// Act:
		const auto& result = score -= ChainScore(0x1000'0002'F000'0010, 0x0200'0500'0030'0005);
		auto scoreArray = score.toArray();

		// Assert:
		EXPECT_EQ(&score, &result);
		EXPECT_EQ(0x7FDE'4264'AC23'D668, scoreArray[0]);
		EXPECT_EQ(0x786B'2F81'0205'43B1, scoreArray[1]);
	}

	// endregion

	// region insertion operator

	TEST(ChainScoreTests, CanOutput64BitChainScoreToStream) {
		// Arrange:
		ChainScore score(1235498201);

		// Act:
		auto str = test::ToString(score);

		// Assert:
		EXPECT_EQ("1235498201", str);
	}

	TEST(ChainScoreTests, CanOutput128BitChainScoreToStream) {
		// Arrange:
		ChainScore score(0x8FDE'4267'9C23'D678, 0x006B'3481'0235'43B6);

		// Act:
		std::stringstream out;
		out << std::hex << score;

		// Assert:
		EXPECT_EQ("8FDE42679C23D678006B3481023543B6", out.str());
	}

	// endregion
}}
