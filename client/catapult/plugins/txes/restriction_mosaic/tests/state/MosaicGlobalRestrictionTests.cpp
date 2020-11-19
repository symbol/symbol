/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "src/state/MosaicGlobalRestriction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicGlobalRestrictionTests

	// region ctor

	TEST(TEST_CLASS, CanCreateRestriction) {
		// Act:
		MosaicGlobalRestriction restriction(MosaicId(123));

		// Assert:
		EXPECT_EQ(MosaicId(123), restriction.mosaicId());

		EXPECT_EQ(0u, restriction.size());
		EXPECT_EQ(std::set<uint64_t>(), restriction.keys());
	}

	// endregion

	// region get

	TEST(TEST_CLASS, CannotGetValueForUnsetRestriction) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::LT });

		// Act:
		MosaicGlobalRestriction::RestrictionRule rule;
		auto result = restriction.tryGet(112, rule);

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, CanGetValueForSetRestriction) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::LT });

		// Act:
		MosaicGlobalRestriction::RestrictionRule rule;
		auto result = restriction.tryGet(111, rule);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(MosaicId(17), rule.ReferenceMosaicId);
		EXPECT_EQ(444u, rule.RestrictionValue);
		EXPECT_EQ(model::MosaicRestrictionType::LT, rule.RestrictionType);
	}

	// endregion

	// region set

	namespace {
		void AssertCanGetValue(const MosaicGlobalRestriction& restriction, uint64_t key, uint64_t expectedValue) {
			MosaicGlobalRestriction::RestrictionRule rule;
			auto result = restriction.tryGet(key, rule);

			EXPECT_TRUE(result) << "for key " << key;
			EXPECT_EQ(expectedValue, rule.RestrictionValue) << "for key " << key;
		}

		void AssertCannotGetValue(const MosaicGlobalRestriction& restriction, uint64_t key) {
			MosaicGlobalRestriction::RestrictionRule rule;
			auto result = restriction.tryGet(key, rule);

			EXPECT_FALSE(result) << "for key " << key;
		}
	}

	TEST(TEST_CLASS, CanSetSingleValue) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));

		// Act:
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::LT });

		// Assert:
		EXPECT_EQ(1u, restriction.size());
		AssertCanGetValue(restriction, 111, 444);
		EXPECT_EQ(std::set<uint64_t>({ 111 }), restriction.keys());
	}

	TEST(TEST_CLASS, CannotSetSentinelValue) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));

		// Act:
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::NONE });

		// Assert:
		EXPECT_EQ(0u, restriction.size());
		AssertCannotGetValue(restriction, 111);
		EXPECT_EQ(std::set<uint64_t>(), restriction.keys());
	}

	TEST(TEST_CLASS, CanSetMultipleValues) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));

		// Act:
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::LT });
		restriction.set(321, { MosaicId(2), 987, model::MosaicRestrictionType::GT });
		restriction.set(222, { MosaicId(3), 567, model::MosaicRestrictionType::EQ });

		// Assert:
		EXPECT_EQ(3u, restriction.size());
		AssertCanGetValue(restriction, 111, 444);
		AssertCanGetValue(restriction, 222, 567);
		AssertCanGetValue(restriction, 321, 987);
		EXPECT_EQ(std::set<uint64_t>({ 111, 222, 321 }), restriction.keys());
	}

	TEST(TEST_CLASS, CanChangeSingleValue) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::LT });
		restriction.set(321, { MosaicId(2), 987, model::MosaicRestrictionType::GT });

		// Act:
		restriction.set(111, { MosaicId(9), 555, model::MosaicRestrictionType::EQ });

		// Assert:
		EXPECT_EQ(2u, restriction.size());
		AssertCanGetValue(restriction, 111, 555);
		AssertCanGetValue(restriction, 321, 987);
		EXPECT_EQ(std::set<uint64_t>({ 111, 321 }), restriction.keys());
	}

	TEST(TEST_CLASS, CanRemoveSingleValue) {
		// Arrange:
		MosaicGlobalRestriction restriction(MosaicId(123));
		restriction.set(111, { MosaicId(17), 444, model::MosaicRestrictionType::LT });
		restriction.set(321, { MosaicId(2), 987, model::MosaicRestrictionType::GT });

		// Act:
		restriction.set(111, { MosaicId(9), 555, model::MosaicRestrictionType::NONE });

		// Assert:
		EXPECT_EQ(1u, restriction.size());
		AssertCannotGetValue(restriction, 111);
		AssertCanGetValue(restriction, 321, 987);
		EXPECT_EQ(std::set<uint64_t>({ 321 }), restriction.keys());
	}

	// endregion
}}
