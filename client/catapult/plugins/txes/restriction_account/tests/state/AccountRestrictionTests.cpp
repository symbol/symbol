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

#include "src/state/AccountRestriction.h"
#include "src/state/AccountRestrictionUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountRestrictionTests

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationAction::Add;
		constexpr size_t Custom_Value_Size = 53;
		constexpr auto Custom_RestrictionAccount_Flags = static_cast<model::AccountRestrictionFlags>(0x78);

		using CustomAccountRestriction = std::array<uint8_t, Custom_Value_Size>;
		using RawAccountRestrictionValue = AccountRestriction::RawValue;
		using RawAccountRestrictionValues = std::vector<RawAccountRestrictionValue>;
		using CustomAccountRestrictionModifications = std::vector<model::AccountRestrictionModification>;

		using OperationType = AccountRestrictionOperationType;

		struct TestContext {
		public:
			explicit TestContext(const std::vector<model::AccountRestrictionModificationAction>& modificationActions) {
				for (auto modificationAction : modificationActions)
					Modifications.push_back({ modificationAction, test::GenerateRandomVector(Custom_Value_Size) });
			}

		public:
			CustomAccountRestrictionModifications Modifications;
		};

		RawAccountRestrictionValues ExtractRawAccountRestrictionValues(const CustomAccountRestrictionModifications& modifications) {
			RawAccountRestrictionValues values;
			for (const auto& modification : modifications)
				values.push_back(modification.Value);

			return values;
		}

		AccountRestriction CreateWithOperationType(
				OperationType operationType,
				const CustomAccountRestrictionModifications& modifications) {
			auto restrictionFlags = OperationType::Allow == operationType
					? Custom_RestrictionAccount_Flags
					: Custom_RestrictionAccount_Flags | model::AccountRestrictionFlags::Block;
			AccountRestriction restriction(restrictionFlags, Custom_Value_Size);
			for (const auto& modification : modifications) {
				if (OperationType::Allow == operationType)
					restriction.allow({ modification.ModificationAction, modification.Value });
				else
					restriction.block({ modification.ModificationAction, modification.Value });
			}

			return restriction;
		}

		// region AllowTraits / BlockTraits

		struct AllowTraits {
			static AccountRestriction Create(const CustomAccountRestrictionModifications& modifications) {
				return CreateWithOperationType(DefaultOperationType(), modifications);
			}

			static AccountRestriction CreateWithOppositeOperationType(const CustomAccountRestrictionModifications& modifications) {
				return CreateWithOperationType(OppositeOperationType(), modifications);
			}

			static OperationType DefaultOperationType() {
				return OperationType::Allow;
			}

			static OperationType OppositeOperationType() {
				return OperationType::Block;
			}

			static bool CanAdd(const AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				return restriction.canAllow({ model::AccountRestrictionModificationAction::Add, value });
			}

			static bool CanRemove(const AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				return restriction.canAllow({ model::AccountRestrictionModificationAction::Del, value });
			}

			static void Add(AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				restriction.allow({ model::AccountRestrictionModificationAction::Add, value });
			}

			static void Remove(AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				restriction.allow({ model::AccountRestrictionModificationAction::Del, value });
			}
		};

		struct BlockTraits {
			static AccountRestriction Create(const CustomAccountRestrictionModifications& modifications) {
				return CreateWithOperationType(DefaultOperationType(), modifications);
			}

			static AccountRestriction CreateWithOppositeOperationType(const CustomAccountRestrictionModifications& modifications) {
				return CreateWithOperationType(OppositeOperationType(), modifications);
			}

			static OperationType DefaultOperationType() {
				return OperationType::Block;
			}

			static OperationType OppositeOperationType() {
				return OperationType::Allow;
			}

			static bool CanAdd(const AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				return restriction.canBlock({ model::AccountRestrictionModificationAction::Add, value });
			}

			static bool CanRemove(const AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				return restriction.canBlock({ model::AccountRestrictionModificationAction::Del, value });
			}

			static void Add(AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				restriction.block({ model::AccountRestrictionModificationAction::Add, value });
			}

			static void Remove(AccountRestriction& restriction, const RawAccountRestrictionValue& value) {
				restriction.block({ model::AccountRestrictionModificationAction::Del, value });
			}
		};

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AllowTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion

		void AssertEqualValues(const RawAccountRestrictionValues& expectedValues, const std::set<RawAccountRestrictionValue>& values) {
			ASSERT_EQ(expectedValues.size(), values.size());

			auto valuesCopy(values);
			auto i = 0u;
			for (const auto& expectedValue : expectedValues) {
				auto iter = valuesCopy.find(expectedValue);
				ASSERT_NE(valuesCopy.cend(), iter) << "at index " << i;

				EXPECT_EQ(expectedValue, *iter) << "at index " << i;
				valuesCopy.erase(iter);
				++i;
			}

			EXPECT_TRUE(valuesCopy.empty());
		}
	}

	TEST(TEST_CLASS, CanCreateAccountRestriction) {
		// Act:
		AccountRestriction restriction(Custom_RestrictionAccount_Flags, Custom_Value_Size);

		// Assert:
		EXPECT_EQ(Custom_RestrictionAccount_Flags, restriction.descriptor().directionalRestrictionFlags());
		EXPECT_EQ(OperationType::Block, restriction.descriptor().operationType());
		EXPECT_EQ(
				model::AccountRestrictionFlags(Custom_RestrictionAccount_Flags | model::AccountRestrictionFlags::Block),
				restriction.descriptor().raw());
		EXPECT_TRUE(restriction.values().empty());
	}

	TEST(TEST_CLASS, AccountRestrictionValueSizeReturnsExpectedSize) {
		// Arrange:
		AccountRestriction restriction(Custom_RestrictionAccount_Flags, Custom_Value_Size);

		// Act + Assert:
		EXPECT_EQ(Custom_Value_Size, restriction.valueSize());
	}

	TRAITS_BASED_TEST(ContainsReturnsTrueWhenValueIsKnown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);
		auto values = ExtractRawAccountRestrictionValues(context.Modifications);

		// Assert:
		for (const auto& value : values)
			EXPECT_TRUE(restriction.contains(value));
	}

	TRAITS_BASED_TEST(ContainsReturnsFalseWhenValueIsUnknown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Assert:
		for (auto i = 0u; i < 10; ++i)
			EXPECT_FALSE(restriction.contains(test::GenerateRandomVector(Custom_Value_Size)));
	}

	TRAITS_BASED_TEST(CanAddValueWhenOperationIsPermissible) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CanAdd(restriction, test::GenerateRandomVector(Custom_Value_Size)));
	}

	TRAITS_BASED_TEST(CannotAddValueWhenValueSizeIsInvalid) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::CanAdd(restriction, test::GenerateRandomVector(Custom_Value_Size - 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::CanAdd(restriction, test::GenerateRandomVector(Custom_Value_Size + 1)), catapult_invalid_argument);
	}

	TRAITS_BASED_TEST(CannotAddValueWhenValueIsKnown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanAdd(restriction, context.Modifications[1].Value));
	}

	TRAITS_BASED_TEST(CanAddValueWhenOperationTypeConflictsAccountRestrictionFlagsButValuesAreEmpty) {
		// Arrange:
		TestContext context({});
		auto restriction = TTraits::CreateWithOppositeOperationType(context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CanAdd(restriction, test::GenerateRandomVector(Custom_Value_Size)));
	}

	TRAITS_BASED_TEST(CannotAddValueWhenOperationTypeConflictsAccountRestrictionFlagsAndValuesAreNotEmpty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::CreateWithOppositeOperationType(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanAdd(restriction, test::GenerateRandomVector(Custom_Value_Size)));
	}

	TRAITS_BASED_TEST(CanRemoveValueWhenOperationIsPermissible) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CanRemove(restriction, context.Modifications[1].Value));
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenValueSizeIsInvalid) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::CanRemove(restriction, test::GenerateRandomVector(Custom_Value_Size - 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::CanRemove(restriction, test::GenerateRandomVector(Custom_Value_Size + 1)), catapult_invalid_argument);
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenValueIsUnknown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanRemove(restriction, test::GenerateRandomVector(Custom_Value_Size)));
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenOperationTypeConflictsAccountRestrictionFlagsAndValuesAreNotEmpty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::CreateWithOppositeOperationType(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanRemove(restriction, context.Modifications[1].Value));
	}

	TRAITS_BASED_TEST(AddAddsValueToSet) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);
		auto value = test::GenerateRandomVector(Custom_Value_Size);

		// Act:
		TTraits::Add(restriction, value);

		// Assert:
		auto expectedValues = ExtractRawAccountRestrictionValues(context.Modifications);
		expectedValues.push_back(value);
		AssertEqualValues(expectedValues, restriction.values());
	}

	TRAITS_BASED_TEST(CanAddRemovedValueToSetAgain) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);
		auto value = context.Modifications[1].Value;
		TTraits::Remove(restriction, value);

		// Sanity:
		EXPECT_EQ(2u, restriction.values().size());
		EXPECT_TRUE(TTraits::CanAdd(restriction, value));

		// Act:
		TTraits::Add(restriction, value);

		// Assert:
		auto expectedValues = ExtractRawAccountRestrictionValues(context.Modifications);
		AssertEqualValues(expectedValues, restriction.values());
	}

	TRAITS_BASED_TEST(CannotAddValueWhenValueSizeMismatches) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::Add(restriction, test::GenerateRandomVector(Custom_Value_Size + 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::Add(restriction, test::GenerateRandomVector(Custom_Value_Size - 1)), catapult_invalid_argument);
	}

	TRAITS_BASED_TEST(RemoveRemovesValueFromSet) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);
		auto value = context.Modifications[1].Value;

		// Act:
		TTraits::Remove(restriction, value);

		// Assert:
		auto expectedValues = ExtractRawAccountRestrictionValues(context.Modifications);
		expectedValues.erase(expectedValues.cbegin() + 1);
		AssertEqualValues(expectedValues, restriction.values());
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenValueSizeMismatches) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto restriction = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::Remove(restriction, test::GenerateRandomVector(Custom_Value_Size + 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::Remove(restriction, test::GenerateRandomVector(Custom_Value_Size - 1)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, AddCanChangeAccountRestrictionDescriptorToOperationTypeAllow) {
		// Arrange:
		auto restriction = BlockTraits::Create({});

		// Sanity:
		EXPECT_EQ(OperationType::Block, restriction.descriptor().operationType());

		// Act:
		AllowTraits::Add(restriction, test::GenerateRandomVector(Custom_Value_Size));

		// Assert: restriction has now operation type allow
		EXPECT_EQ(OperationType::Allow, restriction.descriptor().operationType());
		EXPECT_EQ(1u, restriction.values().size());
	}

	TEST(TEST_CLASS, RemoveCanChangeAccountRestrictionDescriptorToOperationTypeBlock) {
		// Arrange:
		TestContext context({ Add });
		auto restriction = AllowTraits::Create(context.Modifications);
		auto value = context.Modifications[0].Value;

		// Sanity:
		EXPECT_EQ(OperationType::Allow, restriction.descriptor().operationType());

		// Act:
		AllowTraits::Remove(restriction, value);

		// Assert: restriction has now operation type block
		EXPECT_EQ(OperationType::Block, restriction.descriptor().operationType());
		EXPECT_TRUE(restriction.values().empty());
	}
}}
