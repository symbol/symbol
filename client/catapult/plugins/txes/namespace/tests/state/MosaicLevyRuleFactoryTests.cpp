#include "src/state/MosaicLevyRuleFactory.h"
#include "catapult/model/AccountInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicLevyRuleFactoryTests

	// region ctor

	TEST(TEST_CLASS, CanCreateMosaicLevyRuleFactory) {
		// Act:
		MosaicLevyRuleFactory factory;

		// Assert:
		EXPECT_EQ(5u, factory.size());
	}

	// endregion

	namespace {
		constexpr MosaicId Default_Mosaic_Id(123);

		void AssertRule(
				Amount::ValueType lowerBound,
				Amount::ValueType upperBound,
				uint64_t percentile,
				RuleId ruleId,
				const std::vector<Amount::ValueType>& amounts,
				const std::vector<Amount::ValueType>& expectedAmounts) {
			// Assert:
			ASSERT_EQ(expectedAmounts.size(), amounts.size());

			// Arrange:
			MosaicLevyRuleFactory factory;
			auto rule = factory.createRule(Amount(lowerBound), Amount(upperBound), percentile, ruleId);

			// Assert:
			auto i = 0u;
			for (auto amount : amounts) {
				auto mosaic = rule(model::Mosaic{ Default_Mosaic_Id, Amount(amount) });
				EXPECT_EQ(Default_Mosaic_Id, mosaic.MosaicId) << "MosaicId at " << i;
				EXPECT_EQ(expectedAmounts[i], mosaic.Amount.unwrap()) << "Amount at " << i;
				++i;
			}
		}
	}

	// region RuleId::Constant

	TEST(TEST_CLASS, ConstantRule_AlwaysReturnsSameMosaic) {
		// Arrange:
		std::vector <Amount::ValueType> amounts{ 0u, 1u, 10u, 1000u, 10000u, 100000u, 1000000u };
		std::vector<Amount::ValueType> expectedAmounts{ 357u, 357u, 357u, 357u, 357u, 357u, 357u };

		// Assert:
		AssertRule(357, 0, 0, RuleId::Constant, amounts, expectedAmounts);
	}

	// endregion

	// region RuleId::Percentile

	TEST(TEST_CLASS, PercentileRule_AlwaysReturnsMosaicProportionalToAmount) {
		// Arrange: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 0u, 1u, 99u, 100u, 101u, 500u, 567u, 12345u, 987654321u };
		std::vector<Amount::ValueType> expectedAmounts{ 0u, 0u, 0u, 1u, 1u, 5u, 5u, 123u, 9876543u };

		// Assert:
		AssertRule(0, 0, 100, RuleId::Percentile, amounts, expectedAmounts);
	}

	// endregion

	// region RuleId::Lower_Bounded_Percentile

	TEST(TEST_CLASS, LowerBoundedPercentileRule_RespectsLowerBound) {
		// Arrange: lower bound: 12, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 0u, 99u, 100u, 101u, 1000u, 1299u };
		std::vector<Amount::ValueType> expectedAmounts{ 12u, 12u, 12u, 12u, 12u, 12u };

		// Assert:
		AssertRule(12, 0, 100, RuleId::Lower_Bounded_Percentile, amounts, expectedAmounts);
	}

	TEST(TEST_CLASS, LowerBoundedPercentileRule_ReturnsPercentileIfHigherThanLowerBound) {
		// Arrange: lower bound: 12, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 1300u, 2000u, 12345u, 987654321u };
		std::vector<Amount::ValueType> expectedAmounts{ 13u, 20u, 123u, 9876543u };

		// Assert:
		AssertRule(12, 0, 100, RuleId::Lower_Bounded_Percentile, amounts, expectedAmounts);
	}

	// endregion

	// region RuleId::Upper_Bounded_Percentile

	TEST(TEST_CLASS, UpperBoundedPercentileRule_RespectsUpperBound) {
		// Arrange: upper bound: 12, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 1200u, 1201u, 1500u, 5000u, 12345u, 987654321u };
		std::vector<Amount::ValueType> expectedAmounts{ 12u, 12u, 12u, 12u, 12u, 12u };

		// Assert:
		AssertRule(0, 12, 100, RuleId::Upper_Bounded_Percentile, amounts, expectedAmounts);
	}

	TEST(TEST_CLASS, UpperBoundedPercentileRule_ReturnsPercentileIfLowerThanUpperBound) {
		// Arrange: upper bound: 12, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 0u, 1u, 100u, 521u, 1000u, 1199u };
		std::vector<Amount::ValueType> expectedAmounts{ 0u, 0u, 1u, 5u, 10u, 11u };

		// Assert:
		AssertRule(0, 12, 100, RuleId::Upper_Bounded_Percentile, amounts, expectedAmounts);
	}

	// endregion

	// region RuleId::Bounded_Percentile

	TEST(TEST_CLASS, BoundedPercentileRule_RespectsLowerBound) {
		// Arrange: lower bound: 12, upper bound: 20, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 0u, 99u, 100u, 101u, 1000u, 1299u };
		std::vector<Amount::ValueType> expectedAmounts{ 12u, 12u, 12u, 12u, 12u, 12u };

		// Assert:
		AssertRule(12, 20, 100, RuleId::Bounded_Percentile, amounts, expectedAmounts);
	}

	TEST(TEST_CLASS, BoundedPercentileRule_RespectsUpperBound) {
		// Arrange: lower bound: 12, upper bound: 20, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 2000u, 2001u, 4500u, 9000u, 12345u, 987654321u };
		std::vector<Amount::ValueType> expectedAmounts{ 20u, 20u, 20u, 20u, 20u, 20u };

		// Assert:
		AssertRule(12, 20, 100, RuleId::Bounded_Percentile, amounts, expectedAmounts);
	}

	TEST(TEST_CLASS, BoundedPercentileRule_ReturnsPercentileIfBetweenLowerAndUpperBound) {
		// Arrange: lower bound: 12, upper bound: 20, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 1300u, 1301u, 1500u, 2000u, 2099u };
		std::vector<Amount::ValueType> expectedAmounts{ 13u, 13u, 15u, 20u, 20u };

		// Assert:
		AssertRule(12, 20, 100, RuleId::Bounded_Percentile, amounts, expectedAmounts);
	}

	// endregion

	// region edge case

	TEST(TEST_CLASS, BoundedPercentileRule_IsConstantRuleIfLowerBoundIsGreaterThanOrEqualToUpperBound) {
		// Arrange: lower bound: 12, upper bound: 10, percentile: truncated 1% of amount
		std::vector <Amount::ValueType> amounts{ 0u, 1u, 10u, 1000u, 10000u, 100000u, 1000000u };
		std::vector<Amount::ValueType> expectedAmounts{ 10u, 10u, 10u, 10u, 10u, 10u, 10u };

		// Assert:
		AssertRule(12, 10, 100, RuleId::Bounded_Percentile, amounts, expectedAmounts);
	}

	// endregion

	// region unknown rule

	TEST(TEST_CLASS, CreateRuleThrowsIfRuleIsUnknown) {
		// Arrange:
		MosaicLevyRuleFactory factory;
		uint8_t numRules = static_cast<uint8_t>(factory.size());

		// Act + Assert:
		// note that VS complains if curly braces are removed
		for (uint8_t ruleId : { numRules, std::numeric_limits<uint8_t>::max() }) {
			EXPECT_THROW(factory.createRule(Amount(100), Amount(200), 300u, static_cast<RuleId>(ruleId)), catapult_invalid_argument)
					<< "rule id " << ruleId;
		}
	}

	// endregion
}}
