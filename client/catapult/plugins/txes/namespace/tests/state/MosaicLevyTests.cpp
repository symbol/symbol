#include "src/state/MosaicLevy.h"
#include "src/state/MosaicLevyRuleFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	TEST(MosaicLevyTests, CanCreateMosaicLevy) {
		// Arrange:
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		MosaicLevyRuleFactory factory;
		std::vector<MosaicLevyRule> rules;
		rules.push_back(factory.createRule(Amount(), Amount(), 0, RuleId::Lower_Bounded_Percentile));
		rules.push_back(factory.createRule(Amount(), Amount(), 0, RuleId::Constant));

		// Act:
		MosaicLevy levy(MosaicId(123), recipient, rules);

		// Assert:
		EXPECT_EQ(MosaicId(123), levy.id());
		EXPECT_EQ(recipient, levy.recipient());
		EXPECT_EQ(rules.size(), levy.rules().size());
	}
}}
