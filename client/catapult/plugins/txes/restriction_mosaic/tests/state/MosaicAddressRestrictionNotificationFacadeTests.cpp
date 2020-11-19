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

#include "src/state/MosaicAddressRestrictionNotificationFacade.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicAddressRestrictionNotificationFacadeTests

	// region test utils

	namespace {
		using FacadeType = MosaicAddressRestrictionNotificationFacade<static_cast<model::NotificationType>(0)>;

		auto CreateNotification(MosaicId mosaicId, const UnresolvedAddress& targetAddress, uint64_t key, uint64_t value) {
			return FacadeType::NotificationType(test::UnresolveXor(mosaicId), key, targetAddress, value);
		}
	}

	// endregion

	// region uniqueKey

	TEST(TEST_CLASS, UniqueKeyConstructsKeyFromNotification) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto uniqueKey = facade.uniqueKey();

		// Assert:
		Hash256 expectedUniqueKey;
		crypto::Sha3_256_Builder builder;
		builder.update({ reinterpret_cast<const uint8_t*>(&mosaicId), sizeof(MosaicId) });
		builder.update(address);
		builder.final(expectedUniqueKey);

		EXPECT_EQ(expectedUniqueKey, uniqueKey);
	}

	// endregion

	// region isDeleteAction

	namespace {
		bool IsDeleteAction(uint64_t value) {
			// Arrange:
			auto mosaicId = test::GenerateRandomValue<MosaicId>();
			auto unresolvedAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
			auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, value);
			auto resolvers = test::CreateResolverContextXor();
			auto facade = FacadeType(notification, resolvers);

			// Act:
			return facade.isDeleteAction();
		}
	}

	TEST(TEST_CLASS, IsDeleteActionReturnsTrueWhenRestrictionValueIsSentinel) {
		EXPECT_TRUE(IsDeleteAction(MosaicAddressRestriction::Sentinel_Removal_Value));
	}

	TEST(TEST_CLASS, IsDeleteActionReturnsFalseWhenRestrictionValueIsNotSentinel) {
		// Assert: try a subset of values
		for (auto i = 0u; i <= 0xFF; ++i)
			EXPECT_FALSE(IsDeleteAction(i)) << i;
	}

	// endregion

	// region tryGet

	TEST(TEST_CLASS, TryGetReturnsFalseWhenEntryDoesNotContainRule) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		auto restriction = MosaicAddressRestriction(mosaicId, address);
		restriction.set(112, 444);
		auto entry = MosaicRestrictionEntry(restriction);

		// Act:
		uint64_t rule;
		auto result = facade.tryGet(entry, rule);

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, TryGetReturnsTrueWhenEntryContainsRule) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		auto restriction = MosaicAddressRestriction(mosaicId, address);
		restriction.set(111, 444);
		auto entry = MosaicRestrictionEntry(restriction);

		// Act:
		uint64_t rule;
		auto result = facade.tryGet(entry, rule);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(444u, rule);
	}

	// endregion

	// region update

	TEST(TEST_CLASS, UpdateSetsRepresentativeRuleInEntry) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		auto restriction = MosaicAddressRestriction(mosaicId, address);
		restriction.set(112, 444);
		auto entry = MosaicRestrictionEntry(restriction);

		// Sanity:
		EXPECT_EQ(1u, entry.asAddressRestriction().size());

		// Act:
		auto result = facade.update(entry);

		// Assert:
		EXPECT_EQ(2u, result);
		EXPECT_EQ(2u, entry.asAddressRestriction().size());

		auto rule = entry.asAddressRestriction().get(111);
		EXPECT_EQ(222u, rule);
	}

	// endregion

	// region isUnset

	TEST(TEST_CLASS, IsUnsetReturnsTrueWhenRestrictionValueIsUnset) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto unresolvedAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, MosaicAddressRestriction::Sentinel_Removal_Value);
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
		auto unresolvedAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isUnset = facade.isUnset();

		// Assert:
		EXPECT_FALSE(isUnset);
	}

	// endregion

	// region isMatch

	TEST(TEST_CLASS, IsMatchReturnsTrueWhenRestrictionValueMatches) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto unresolvedAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isMatch = facade.isMatch(222);

		// Assert:
		EXPECT_TRUE(isMatch);
	}

	TEST(TEST_CLASS, IsMatchReturnsFalseWhenRestrictionValueIsDifferent) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto unresolvedAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto isMatch = facade.isMatch(223);

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	// endregion

	// region toRule

	TEST(TEST_CLASS, ToRuleReturnsRepresentativeRule) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto unresolvedAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto rule = facade.toRule();

		// Assert:
		EXPECT_EQ(222u, rule);
	}

	// endregion

	// region toRestriction

	TEST(TEST_CLASS, ToRestrictionReturnsRepresentativeRestriction) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		auto notification = CreateNotification(mosaicId, unresolvedAddress, 111, 222);
		auto resolvers = test::CreateResolverContextXor();
		auto facade = FacadeType(notification, resolvers);

		// Act:
		auto restriction = facade.toRestriction();

		// Assert:
		EXPECT_EQ(mosaicId, restriction.mosaicId());
		EXPECT_EQ(address, restriction.address());
		EXPECT_EQ(0u, restriction.size());
	}

	// endregion
}}
