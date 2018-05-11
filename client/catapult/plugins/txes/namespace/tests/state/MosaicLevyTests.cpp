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

#include "src/state/MosaicLevy.h"
#include "src/state/MosaicLevyRuleFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicLevyTests

	TEST(TEST_CLASS, CanCreateMosaicLevy) {
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
