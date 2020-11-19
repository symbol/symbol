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

#include "src/state/MosaicGlobalRestrictionNotificationFacade.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicGlobalRestrictionNotificationFacadeTests

	// region test utils

	namespace {
		using FacadeType = MosaicGlobalRestrictionNotificationFacade<static_cast<model::NotificationType>(0)>;

		auto CreateNotification(
				MosaicId mosaicId,
				MosaicId referenceMosaicId,
				uint64_t key,
				uint64_t value,
				model::MosaicRestrictionType restrictionType = model::MosaicRestrictionType::NONE) {
			return FacadeType::NotificationType(
					test::UnresolveXor(mosaicId),
					test::UnresolveXor(referenceMosaicId),
					key,
					value,
					restrictionType);
		}
	}

	// endregion

	// region uniqueKey

	TEST(TEST_CLASS, UniqueKeyConstructsKeyFromNotification) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto uniqueKey = facade.uniqueKey();

		// Assert:
		Hash256 expectedUniqueKey;
		crypto::Sha3_256_Builder builder;
		builder.update({ reinterpret_cast<const uint8_t*>(&mosaicId), sizeof(MosaicId) });
		builder.update(Address());
		builder.final(expectedUniqueKey);

		EXPECT_EQ(expectedUniqueKey, uniqueKey);
	}

	// endregion

	// region isDeleteAction

	namespace {
		bool IsDeleteAction(model::MosaicRestrictionType restrictionType) {
			// Arrange:
			auto mosaicId = test::GenerateRandomValue<MosaicId>();
			auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
			auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, restrictionType);
			auto resolvers = test::CreateResolverContextXor();
			auto facade = FacadeType(notification, resolvers);

			// Act:
			return facade.isDeleteAction();
		}
	}

	TEST(TEST_CLASS, IsDeleteActionReturnsTrueWhenRestrictionTypeIsNone) {
		EXPECT_TRUE(IsDeleteAction(model::MosaicRestrictionType::NONE));
	}

	TEST(TEST_CLASS, IsDeleteActionReturnsFalseWhenRestrictionTypeIsNotNone) {
		for (auto i = 1u; i <= 0xFF; ++i)
			EXPECT_FALSE(IsDeleteAction(static_cast<model::MosaicRestrictionType>(i))) << i;
	}

	// endregion

	// region tryGet

	TEST(TEST_CLASS, TryGetReturnsFalseWhenEntryDoesNotContainRule) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		auto restriction = MosaicGlobalRestriction(mosaicId);
		restriction.set(112, { MosaicId(987), 444, model::MosaicRestrictionType::LT });
		auto entry = MosaicRestrictionEntry(restriction);

		// Act:
		MosaicGlobalRestriction::RestrictionRule rule;
		auto result = facade.tryGet(entry, rule);

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, TryGetReturnsTrueWhenEntryContainsRule) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		auto restriction = MosaicGlobalRestriction(mosaicId);
		restriction.set(111, { MosaicId(987), 444, model::MosaicRestrictionType::LT });
		auto entry = MosaicRestrictionEntry(restriction);

		// Act:
		MosaicGlobalRestriction::RestrictionRule rule;
		auto result = facade.tryGet(entry, rule);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(MosaicId(987), rule.ReferenceMosaicId);
		EXPECT_EQ(444u, rule.RestrictionValue);
		EXPECT_EQ(model::MosaicRestrictionType::LT, rule.RestrictionType);
	}

	// endregion

	// region update

	TEST(TEST_CLASS, UpdateSetsRepresentativeRuleInEntry) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, model::MosaicRestrictionType::GT);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		auto restriction = MosaicGlobalRestriction(mosaicId);
		restriction.set(112, { MosaicId(987), 444, model::MosaicRestrictionType::LT });
		auto entry = MosaicRestrictionEntry(restriction);

		// Sanity:
		EXPECT_EQ(1u, entry.asGlobalRestriction().size());

		// Act:
		auto result = facade.update(entry);

		// Assert:
		EXPECT_EQ(2u, result);
		EXPECT_EQ(2u, entry.asGlobalRestriction().size());

		MosaicGlobalRestriction::RestrictionRule rule;
		entry.asGlobalRestriction().tryGet(111, rule);
		EXPECT_EQ(referenceMosaicId, rule.ReferenceMosaicId);
		EXPECT_EQ(222u, rule.RestrictionValue);
		EXPECT_EQ(model::MosaicRestrictionType::GT, rule.RestrictionType);
	}

	// endregion

	// region isUnset

	TEST(TEST_CLASS, IsUnsetReturnsTrueWhenAllValuesAreUnset) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 0);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isUnset = facade.isUnset();

		// Assert:
		EXPECT_TRUE(isUnset);
	}

	TEST(TEST_CLASS, IsUnsetReturnsFalseWhenRestrictionValueIsSet) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isUnset = facade.isUnset();

		// Assert:
		EXPECT_FALSE(isUnset);
	}

	TEST(TEST_CLASS, IsUnsetReturnsFalseWhenRestrictionTypeIsSet) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 0, model::MosaicRestrictionType::EQ);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isUnset = facade.isUnset();

		// Assert:
		EXPECT_FALSE(isUnset);
	}

	// endregion

	// region isMatch

	TEST(TEST_CLASS, IsMatchReturnsTrueWhenAllValuesMatch) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, model::MosaicRestrictionType::LT);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isMatch = facade.isMatch({ referenceMosaicId, 222, model::MosaicRestrictionType::LT });

		// Assert:
		EXPECT_TRUE(isMatch);
	}

	TEST(TEST_CLASS, IsMatchReturnsFalseWhenReferenceMosaicIdIsDifferent) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, model::MosaicRestrictionType::LT);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isMatch = facade.isMatch({ test::GenerateRandomValue<MosaicId>(), 222, model::MosaicRestrictionType::LT });

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	TEST(TEST_CLASS, IsMatchReturnsFalseWhenRestrictionValueIsDifferent) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, model::MosaicRestrictionType::LT);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isMatch = facade.isMatch({ referenceMosaicId, 223, model::MosaicRestrictionType::LT });

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	TEST(TEST_CLASS, IsMatchReturnsFalseWhenRestrictionTypeIsDifferent) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, model::MosaicRestrictionType::LT);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isMatch = facade.isMatch({ referenceMosaicId, 222, model::MosaicRestrictionType::LE });

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	// endregion

	// region toRule

	TEST(TEST_CLASS, ToRuleReturnsRepresentativeRule) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222, model::MosaicRestrictionType::LT);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto rule = facade.toRule();

		// Assert:
		EXPECT_EQ(referenceMosaicId, rule.ReferenceMosaicId);
		EXPECT_EQ(222u, rule.RestrictionValue);
		EXPECT_EQ(model::MosaicRestrictionType::LT, rule.RestrictionType);
	}

	// endregion

	// region toRestriction

	TEST(TEST_CLASS, ToRestrictionReturnsRepresentativeRestriction) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
		auto notification = CreateNotification(mosaicId, referenceMosaicId, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto restriction = facade.toRestriction();

		// Assert:
		EXPECT_EQ(mosaicId, restriction.mosaicId());
		EXPECT_EQ(0u, restriction.size());
	}

	// endregion
}}
