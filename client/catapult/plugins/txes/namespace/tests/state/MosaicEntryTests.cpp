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

#include "src/state/MosaicEntry.h"
#include "src/state/MosaicLevyRuleFactory.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicEntryTests

	// region ctor

	TEST(TEST_CLASS, CanCreateMosaicEntry) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));

		// Act:
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);

		// Assert:
		EXPECT_EQ(NamespaceId(111), entry.namespaceId());
		EXPECT_EQ(MosaicId(225), entry.mosaicId());
		EXPECT_EQ(Height(123), entry.definition().height());
		EXPECT_EQ(Amount(), entry.supply());
		EXPECT_FALSE(entry.hasLevy());
	}

	// endregion

	// region supply

	TEST(TEST_CLASS, CanIncreaseSupply) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act:
		entry.increaseSupply(Amount(321));

		// Assert:
		EXPECT_EQ(Amount(432 + 321), entry.supply());
	}

	TEST(TEST_CLASS, CanDecreaseSupply) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act:
		entry.decreaseSupply(Amount(321));

		// Assert:
		EXPECT_EQ(Amount(432 - 321), entry.supply());
	}

	TEST(TEST_CLASS, CanDecreaseSupplyToZero) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act:
		entry.decreaseSupply(Amount(432));

		// Assert:
		EXPECT_EQ(Amount(), entry.supply());
	}

	TEST(TEST_CLASS, CannotDecreaseSupplyBelowZero) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act + Assert:
		EXPECT_THROW(entry.decreaseSupply(Amount(433)), catapult_invalid_argument);
	}

	// endregion

	// region levy

	namespace {
		auto CreateRule(RuleId ruleId) {
			MosaicLevyRuleFactory factory;
			return factory.createRule(Amount(), Amount(), 0u, ruleId);
		}
	}

	TEST(TEST_CLASS, CannotAccessLevyIfNoneIsPresent) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);

		// Sanity:
		EXPECT_FALSE(entry.hasLevy());

		// Act + Assert:
		EXPECT_THROW(entry.levy(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSetLevyIfNoneIsPresent) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto rules = std::vector<MosaicLevyRule>{ CreateRule(RuleId::Bounded_Percentile), CreateRule(RuleId::Constant) };
		auto pLevy = std::make_unique<MosaicLevy>(MosaicId(234), recipient, rules);

		// Sanity:
		EXPECT_FALSE(entry.hasLevy());

		// Act:
		entry.setLevy(std::move(pLevy));

		// Assert:
		ASSERT_TRUE(entry.hasLevy());

		const auto& levy = entry.levy();
		EXPECT_EQ(MosaicId(234), levy.id());
		EXPECT_EQ(recipient, levy.recipient());
		EXPECT_EQ(2u, levy.rules().size());
	}

	TEST(TEST_CLASS, CannotSetLevyIfAlreadyPresent) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(NamespaceId(111), MosaicId(225), definition);
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto pLevy = std::make_unique<MosaicLevy>(MosaicId(234), recipient, std::vector<MosaicLevyRule>());
		auto pLevy2 = std::make_unique<MosaicLevy>(MosaicId(345), recipient, std::vector<MosaicLevyRule>());
		entry.setLevy(std::move(pLevy));

		// Sanity:
		EXPECT_TRUE(entry.hasLevy());

		// Act + Assert:
		EXPECT_THROW(entry.setLevy(std::move(pLevy2)), catapult_runtime_error);
	}

	// endregion
}}
