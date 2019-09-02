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

#include "src/state/TypedAccountRestriction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS TypedAccountRestrictionTests

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationAction::Add;
		constexpr auto Del = model::AccountRestrictionModificationAction::Del;

		constexpr size_t Custom_Value_Size = 53;
		constexpr auto Custom_Restriction_Type = static_cast<model::AccountRestrictionType>(0x78);
		using CustomAccountRestriction = std::array<uint8_t, Custom_Value_Size>;
		using CustomAccountRestrictionModifications = std::vector<model::AccountRestrictionModification<CustomAccountRestriction>>;

		using OperationType = AccountRestrictionOperationType;

		struct TestContext {
		public:
			explicit TestContext(const std::vector<model::AccountRestrictionModificationAction>& modificationActions) {
				for (auto modificationAction : modificationActions)
					Modifications.push_back({ modificationAction, test::GenerateRandomArray<Custom_Value_Size>() });
			}

		public:
			CustomAccountRestrictionModifications Modifications;
		};

		TypedAccountRestriction<CustomAccountRestriction> CreateTypedAccountRestrictionWithValues(
				AccountRestriction& restriction,
				AccountRestrictionOperationType operationType,
				const CustomAccountRestrictionModifications& modifications) {
			for (const auto& modification : modifications) {
				if (AccountRestrictionOperationType::Allow == operationType)
					restriction.allow({ modification.ModificationAction, ToVector(modification.Value) });
				else
					restriction.block({ modification.ModificationAction, ToVector(modification.Value) });
			}

			return TypedAccountRestriction<CustomAccountRestriction>(restriction);
		}
	}

	TEST(TEST_CLASS, CanCreateTypedAccountRestrictionAroundAccountRestriction) {
		// Arrange:
		auto rawAccountRestriction = AccountRestriction(Custom_Restriction_Type, 1234);

		// Act:
		auto restriction = TypedAccountRestriction<CustomAccountRestriction>(rawAccountRestriction);

		// Assert:
		EXPECT_EQ(
				model::AccountRestrictionType(Custom_Restriction_Type | model::AccountRestrictionType::Block),
				restriction.descriptor().raw());
		EXPECT_EQ(0u, restriction.size());
	}

	TEST(TEST_CLASS, ContainsDelegatesToUnderlyingAccountRestriction) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto rawAccountRestriction = AccountRestriction(Custom_Restriction_Type, Custom_Value_Size);
		auto restriction = CreateTypedAccountRestrictionWithValues(rawAccountRestriction, OperationType::Allow, context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(restriction.contains(context.Modifications[1].Value));
		EXPECT_FALSE(restriction.contains(test::GenerateRandomArray<Custom_Value_Size>()));
	}

	TEST(TEST_CLASS, CanAllowDelegatesToUnderlyingAccountRestriction) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto rawAccountRestriction = AccountRestriction(Custom_Restriction_Type, Custom_Value_Size);
		auto restriction = CreateTypedAccountRestrictionWithValues(rawAccountRestriction, OperationType::Allow, context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(restriction.canAllow({ Add, test::GenerateRandomArray<Custom_Value_Size>() }));
		EXPECT_FALSE(restriction.canAllow({ Add, context.Modifications[1].Value }));
		EXPECT_TRUE(restriction.canAllow({ Del, context.Modifications[1].Value }));
		EXPECT_FALSE(restriction.canAllow({ Del, test::GenerateRandomArray<Custom_Value_Size>() }));
	}

	TEST(TEST_CLASS, CanBlockDelegatesToUnderlyingAccountRestriction) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto rawAccountRestriction = AccountRestriction(Custom_Restriction_Type, Custom_Value_Size);
		auto restriction = CreateTypedAccountRestrictionWithValues(rawAccountRestriction, OperationType::Block, context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(restriction.canBlock({ Add, test::GenerateRandomArray<Custom_Value_Size>() }));
		EXPECT_FALSE(restriction.canBlock({ Add, context.Modifications[1].Value }));
		EXPECT_TRUE(restriction.canBlock({ Del, context.Modifications[1].Value }));
		EXPECT_FALSE(restriction.canBlock({ Del, test::GenerateRandomArray<Custom_Value_Size>() }));
	}
}}
