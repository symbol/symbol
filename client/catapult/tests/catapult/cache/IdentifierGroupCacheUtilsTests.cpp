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

#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "tests/test/cache/TestCacheTypes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS IdentifierGroupCacheUtilsTests

	// region test utils

	namespace {
		using TestIdentifierGroup = test::TestCacheTypes::TestIdentifierGroup;
		using TestCacheDescriptor = test::TestCacheTypes::TestActivityCacheDescriptor;

		using HeightGroupedBaseSetType = test::TestCacheTypes::HeightGroupedBaseSetType;
		using BaseSetType = test::TestCacheTypes::BaseActivitySetType;

		TestIdentifierGroup AddValues(TestIdentifierGroup&& group, std::initializer_list<int> values) {
			for (auto value : values)
				group.add(value);

			return std::move(group);
		}
	}

	// endregion

	// region AddIdentifierWithGroup

	namespace {
		template<typename TAction>
		void RunAddIdentifierWithGroupTest(TAction action) {
			// Arrange:
			HeightGroupedBaseSetType groupedSet;
			auto pGroupedDelta = groupedSet.rebase();
			pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
			pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

			// Act + Assert:
			action(*pGroupedDelta);
		}
	}

	TEST(TEST_CLASS, AddIdentifierWithGroup_AddsIdentifierToNewGroup) {
		// Arrange:
		RunAddIdentifierWithGroupTest([](auto& groupedDelta) {
			// Sanity:
			EXPECT_FALSE(groupedDelta.contains(Height(5)));

			// Act:
			AddIdentifierWithGroup(groupedDelta, Height(5), 17);

			// Assert:
			const auto* pGroup = groupedDelta.find(Height(5)).get();
			ASSERT_TRUE(!!pGroup);
			EXPECT_EQ(TestIdentifierGroup::Identifiers({ 17 }), pGroup->identifiers());
		});
	}

	TEST(TEST_CLASS, AddIdentifierWithGroup_AddsIdentifierToExistingGroup) {
		// Arrange:
		RunAddIdentifierWithGroupTest([](auto& groupedDelta) {
			// Act:
			AddIdentifierWithGroup(groupedDelta, Height(3), 7);

			// Assert:
			const auto* pGroup = groupedDelta.find(Height(3)).get();
			ASSERT_TRUE(!!pGroup);
			EXPECT_EQ(TestIdentifierGroup::Identifiers({ 1, 4, 7, 9 }), pGroup->identifiers());
		});
	}

	TEST(TEST_CLASS, AddIdentifierWithGroup_HasNoEffectWhenAddingExistingIdentifierToExistingGroup) {
		// Arrange:
		RunAddIdentifierWithGroupTest([](auto& groupedDelta) {
			// Act:
			AddIdentifierWithGroup(groupedDelta, Height(3), 4);

			// Assert:
			const auto* pGroup = groupedDelta.find(Height(3)).get();
			ASSERT_TRUE(!!pGroup);
			EXPECT_EQ(TestIdentifierGroup::Identifiers({ 1, 4, 9 }), pGroup->identifiers());
		});
	}

	// endregion

	// region RunHeightGroupedTest

	namespace {
		template<typename TAction>
		void RunHeightGroupedTest(Height deactivateHeight, TAction action) {
			// Arrange:
			BaseSetType set;
			auto pDelta = set.rebase();
			pDelta->insert(TestCacheDescriptor::ValueType("a", deactivateHeight));
			pDelta->insert(TestCacheDescriptor::ValueType("xyz", deactivateHeight));
			pDelta->insert(TestCacheDescriptor::ValueType("bbbb", deactivateHeight));
			pDelta->insert(TestCacheDescriptor::ValueType(std::string(9, 'c'), deactivateHeight));
			pDelta->insert(TestCacheDescriptor::ValueType(std::string(100, 'z'), deactivateHeight));

			HeightGroupedBaseSetType groupedSet;
			auto pGroupedDelta = groupedSet.rebase();
			pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 99, 98 }));
			pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 100, 7, 4 }));
			pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(6)), { 1, 3, 9 }));
			pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

			// Act + Assert:
			action(*pDelta, *pGroupedDelta);
		}

		template<typename TAction>
		void RunHeightGroupedTest(TAction action) {
			RunHeightGroupedTest(Height(std::numeric_limits<Height::ValueType>::max()), action);
		}

		size_t CountIdentifiers(const test::TestCacheTypes::HeightGroupedBaseSetDeltaType& groupedDelta, const std::vector<Height>& keys) {
			size_t count = 0;
			for (auto key : keys)
				count += groupedDelta.find(key).get()->size();

			return count;
		}
	}

	// endregion

	// region ForEachIdentifierWithGroup

	TEST(TEST_CLASS, ForEachIdentifierWithGroup_DoesNotCallActionWhenNoIdentifiersInGroup) {
		// Arrange:
		RunHeightGroupedTest([](const auto& delta, const auto& groupedDelta) {
			// Sanity:
			EXPECT_FALSE(groupedDelta.contains(Height(5)));

			// Act:
			auto numActionCalls = 0u;
			ForEachIdentifierWithGroup(delta, groupedDelta, Height(5), [&numActionCalls](const auto&) {
				++numActionCalls;
			});

			// Assert:
			EXPECT_EQ(0u, numActionCalls);
		});
	}

	TEST(TEST_CLASS, ForEachIdentifierWithGroup_CallsActionForAllValuesInGroup) {
		// Arrange:
		RunHeightGroupedTest([](const auto& delta, const auto& groupedDelta) {
			// Act:
			auto numActionCalls = 0u;
			std::unordered_set<std::string> values;
			ForEachIdentifierWithGroup(delta, groupedDelta, Height(6), [&numActionCalls, &values](const auto& value) {
				++numActionCalls;
				values.insert(value.str());
			});

			// Assert:
			EXPECT_EQ(3u, numActionCalls);

			EXPECT_EQ(3u, values.size());
			EXPECT_CONTAINS(values, "a");
			EXPECT_CONTAINS(values, "xyz");
			EXPECT_CONTAINS(values, std::string(9, 'c'));
		});
	}

	TEST(TEST_CLASS, ForEachIdentifierWithGroup_CallsActionForAllValuesInGroupAndIgnoresUnknownValues) {
		// Arrange:
		RunHeightGroupedTest([](const auto& delta, const auto& groupedDelta) {
			// Act:
			auto numActionCalls = 0u;
			std::unordered_set<std::string> values;
			ForEachIdentifierWithGroup(delta, groupedDelta, Height(3), [&numActionCalls, &values](const auto& value) {
				++numActionCalls;
				values.insert(value.str());
			});

			// Assert: value with id 7 is in group but not in underlying set
			EXPECT_EQ(2u, numActionCalls);

			EXPECT_EQ(2u, values.size());
			EXPECT_CONTAINS(values, "bbbb");
			EXPECT_CONTAINS(values, std::string(100, 'z'));
		});
	}

	// endregion

	// region RemoveIdentifierWithGroup

	TEST(TEST_CLASS, RemoveIdentifierWithGroup_DoesNotRemoveAnythingWhenGroupDoesNotExist) {
		// Arrange:
		RunHeightGroupedTest([](const auto&, auto& groupedDelta) {
			// Sanity:
			EXPECT_FALSE(groupedDelta.contains(Height(5)));
			EXPECT_EQ(4u, groupedDelta.size());
			EXPECT_EQ(10u, CountIdentifiers(groupedDelta, { Height(1), Height(3), Height(6), Height(7) }));

			// Act:
			RemoveIdentifierWithGroup(groupedDelta, Height(5), 100);

			// Assert: nothing changed
			EXPECT_EQ(4u, groupedDelta.size());
			EXPECT_EQ(10u, CountIdentifiers(groupedDelta, { Height(1), Height(3), Height(6), Height(7) }));
		});
	}

	TEST(TEST_CLASS, RemoveIdentifierWithGroup_RemovesIdentifierInGroup) {
		// Arrange:
		RunHeightGroupedTest([](const auto&, auto& groupedDelta) {
			// Sanity:
			EXPECT_EQ(4u, groupedDelta.size());
			EXPECT_EQ(10u, CountIdentifiers(groupedDelta, { Height(1), Height(3), Height(6), Height(7) }));

			// Act:
			RemoveIdentifierWithGroup(groupedDelta, Height(3), 100);

			// Assert: identifier was removed
			EXPECT_EQ(4u, groupedDelta.size());
			EXPECT_EQ(9u, CountIdentifiers(groupedDelta, { Height(1), Height(3), Height(6), Height(7) }));
		});
	}

	TEST(TEST_CLASS, RemoveIdentifierWithGroup_RemovesEmptyGroup) {
		// Arrange:
		RunHeightGroupedTest([](const auto&, auto& groupedDelta) {
			// Sanity:
			EXPECT_EQ(4u, groupedDelta.size());
			EXPECT_EQ(10u, CountIdentifiers(groupedDelta, { Height(1), Height(3), Height(6), Height(7) }));

			// Act:
			RemoveIdentifierWithGroup(groupedDelta, Height(1), 99);
			RemoveIdentifierWithGroup(groupedDelta, Height(1), 98);

			// Assert: group was removed
			EXPECT_FALSE(groupedDelta.contains(Height(1)));
			EXPECT_EQ(3u, groupedDelta.size());
			EXPECT_EQ(8u, CountIdentifiers(groupedDelta, { Height(3), Height(6), Height(7) }));
		});
	}

	// endregion

	// region FindDeactivatingIdentifiersAtHeight

	TEST(TEST_CLASS, FindDeactivatingIdentifiersAtHeight_ReturnsNothingWhenNoIdentifiersInGroup) {
		// Arrange:
		RunHeightGroupedTest([](auto& delta, auto& groupedDelta) {
			// Sanity:
			EXPECT_FALSE(groupedDelta.contains(Height(5)));

			// Act:
			auto identifiers = FindDeactivatingIdentifiersAtHeight(delta, groupedDelta, Height(5));

			// Assert: nothing was found
			EXPECT_TRUE(identifiers.empty());
		});
	}

	TEST(TEST_CLASS, FindDeactivatingIdentifiersAtHeight_ReturnsAllValuesInGroupThatDeactivateAtHeight) {
		// Arrange:
		RunHeightGroupedTest(Height(6), [](auto& delta, auto& groupedDelta) {
			// Act:
			auto identifiers = FindDeactivatingIdentifiersAtHeight(delta, groupedDelta, Height(6));

			// Assert:
			EXPECT_EQ(3u, identifiers.size());
			EXPECT_CONTAINS(identifiers, 1);
			EXPECT_CONTAINS(identifiers, 3);
			EXPECT_CONTAINS(identifiers, 9);
		});
	}

	TEST(TEST_CLASS, FindDeactivatingIdentifiersAtHeight_ReturnsNothingWhenAllValuesInGroupStayActive) {
		// Arrange:
		RunHeightGroupedTest(Height(7), [](auto& delta, auto& groupedDelta) {
			// Act:
			auto identifiers = FindDeactivatingIdentifiersAtHeight(delta, groupedDelta, Height(6));

			// Assert: even though there are identifiers at Height(6) because they are active at 5 + 6, none should be returned
			EXPECT_TRUE(identifiers.empty());
		});
	}

	TEST(TEST_CLASS, FindDeactivatingIdentifiersAtHeight_ReturnsNothingWhenAllValuesInGroupStayInactive) {
		// Arrange:
		RunHeightGroupedTest(Height(5), [](auto& delta, auto& groupedDelta) {
			// Act:
			auto identifiers = FindDeactivatingIdentifiersAtHeight(delta, groupedDelta, Height(6));

			// Assert: even though there are identifiers at Height(6) because they are inactive at 5 + 6, none should be returned
			EXPECT_TRUE(identifiers.empty());
		});
	}

	TEST(TEST_CLASS, FindDeactivatingIdentifiersAtHeight_ReturnsAllValuesInGroupThatDeactivateAtHeightAndIgnoresUnknownValues) {
		// Arrange:
		RunHeightGroupedTest(Height(3), [](auto& delta, auto& groupedDelta) {
			// Act:
			auto identifiers = FindDeactivatingIdentifiersAtHeight(delta, groupedDelta, Height(3));

			// Assert:
			EXPECT_EQ(2u, identifiers.size());
			EXPECT_CONTAINS(identifiers, 4);
			EXPECT_CONTAINS(identifiers, 100);
		});
	}

	// endregion
}}
