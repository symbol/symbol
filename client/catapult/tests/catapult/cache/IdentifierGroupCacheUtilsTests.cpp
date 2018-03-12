#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS IdentifierGroupCacheUtilsTests

	namespace {
		// int grouped by Height
		using TestIdentifierGroup = utils::IdentifierGroup<int, Height, std::hash<int>>;

		TestIdentifierGroup AddValues(TestIdentifierGroup&& group, std::initializer_list<int> values) {
			for (auto value : values)
				group.add(value);

			return group;
		}

		struct TestHeightGroupedCacheDescriptor {
			using KeyType = Height;
			using ValueType = TestIdentifierGroup;

			static KeyType GetKeyFromValue(const ValueType& value) {
				return value.key();
			}
		};

		using HeightGroupedTypes = MutableUnorderedMapAdapter<TestHeightGroupedCacheDescriptor, utils::BaseValueHasher<Height>>;
		using HeightGroupedBaseSetType = HeightGroupedTypes::BaseSetType;

		// fake cache that indexes strings by size
		struct TestCacheDescriptor {
			using KeyType = int;
			using ValueType = std::string;

			static KeyType GetKeyFromValue(const ValueType& value) {
				return static_cast<int>(value.size());
			}
		};

		using BasicTypes = MutableUnorderedMapAdapter<TestCacheDescriptor>;
		using BaseSetType = BasicTypes::BaseSetType;
	}

	// region AddIdentifierWithGroup

	TEST(TEST_CLASS, AddIdentifierWithGroup_AddsIdentifierToNewGroup) {
		// Arrange:
		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Sanity:
		EXPECT_FALSE(pGroupedDelta->contains(Height(5)));

		// Act:
		AddIdentifierWithGroup(*pGroupedDelta, Height(5), 17);

		// Assert:
		const auto* pGroup = pGroupedDelta->find(Height(5));
		ASSERT_TRUE(!!pGroup);
		EXPECT_EQ(TestIdentifierGroup::Identifiers({ 17 }), pGroup->identifiers());
	}

	TEST(TEST_CLASS, AddIdentifierWithGroup_AddsIdentifierToExistingGroup) {
		// Arrange:
		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Act:
		AddIdentifierWithGroup(*pGroupedDelta, Height(3), 7);

		// Assert:
		const auto* pGroup = pGroupedDelta->find(Height(3));
		ASSERT_TRUE(!!pGroup);
		EXPECT_EQ(TestIdentifierGroup::Identifiers({ 1, 4, 7, 9 }), pGroup->identifiers());
	}

	TEST(TEST_CLASS, AddIdentifierWithGroup_HasNoEffectWhenAddingExistingIdentifierToExistingGroup) {
		// Arrange:
		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Act:
		AddIdentifierWithGroup(*pGroupedDelta, Height(3), 4);

		// Assert:
		const auto* pGroup = pGroupedDelta->find(Height(3));
		ASSERT_TRUE(!!pGroup);
		EXPECT_EQ(TestIdentifierGroup::Identifiers({ 1, 4, 9 }), pGroup->identifiers());
	}

	// endregion

	// region ForEachIdentifierWithGroup

	TEST(TEST_CLASS, ForEachIdentifierWithGroup_DoesNotCallActionWhenNoIdentifiersInGroup) {
		// Arrange:
		BaseSetType set;
		auto pDelta = set.rebase();
		pDelta->insert("a");
		pDelta->insert(std::string(25, 'z'));

		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 100 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Sanity:
		EXPECT_FALSE(pGroupedDelta->contains(Height(5)));

		// Act:
		auto numActionCalls = 0u;
		ForEachIdentifierWithGroup(*pDelta, *pGroupedDelta, Height(5), [&numActionCalls](const auto&) {
			++numActionCalls;
		});

		// Assert:
		EXPECT_EQ(0u, numActionCalls);
	}

	TEST(TEST_CLASS, ForEachIdentifierWithGroup_CallsActionForAllValuesInGroup) {
		// Arrange:
		BaseSetType set;
		auto pDelta = set.rebase();
		pDelta->insert("a");
		pDelta->insert("bbbb");
		pDelta->insert(std::string(9, 'c'));
		pDelta->insert(std::string(100, 'z'));

		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 100 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Act:
		auto numActionCalls = 0u;
		std::unordered_set<std::string> values;
		ForEachIdentifierWithGroup(*pDelta, *pGroupedDelta, Height(3), [&numActionCalls, &values](const auto& str) {
			++numActionCalls;
			values.insert(str);
		});

		// Assert:
		EXPECT_EQ(3u, numActionCalls);

		EXPECT_EQ(3u, values.size());
		EXPECT_TRUE(values.cend() != values.find("a"));
		EXPECT_TRUE(values.cend() != values.find("bbbb"));
		EXPECT_TRUE(values.cend() != values.find(std::string(9, 'c')));
	}

	TEST(TEST_CLASS, ForEachIdentifierWithGroup_CallsActionForAllValuesInGroupAndIgnoresUnknownValues) {
		// Arrange:
		BaseSetType set;
		auto pDelta = set.rebase();
		pDelta->insert("a");
		pDelta->insert(std::string(9, 'c'));
		pDelta->insert(std::string(100, 'z'));

		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 100 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Act:
		auto numActionCalls = 0u;
		std::unordered_set<std::string> values;
		ForEachIdentifierWithGroup(*pDelta, *pGroupedDelta, Height(3), [&numActionCalls, &values](const auto& str) {
			++numActionCalls;
			values.insert(str);
		});

		// Assert: value with id 4 is in group but not in underlying set
		EXPECT_EQ(2u, numActionCalls);

		EXPECT_EQ(2u, values.size());
		EXPECT_TRUE(values.cend() != values.find("a"));
		EXPECT_TRUE(values.cend() != values.find(std::string(9, 'c')));
	}

	// endregion

	// region RemoveAllIdentifiersWithGroup

	TEST(TEST_CLASS, RemoveAllIdentifiersWithGroup_DoesNotRemoveAnythingWhenNoIdentifiersInGroup) {
		// Arrange:
		BaseSetType set;
		auto pDelta = set.rebase();
		pDelta->insert("a");
		pDelta->insert(std::string(25, 'z'));

		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 100 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Sanity:
		EXPECT_FALSE(pGroupedDelta->contains(Height(5)));

		// Act:
		RemoveAllIdentifiersWithGroup(*pDelta, *pGroupedDelta, Height(5));

		// Assert: nothing was removed
		EXPECT_EQ(2u, pDelta->size());
		EXPECT_EQ(3u, pGroupedDelta->size());
	}

	TEST(TEST_CLASS, RemoveAllIdentifiersWithGroup_RemovesAllValuesInGroup) {
		// Arrange:
		BaseSetType set;
		auto pDelta = set.rebase();
		pDelta->insert("a");
		pDelta->insert("bbbb");
		pDelta->insert(std::string(9, 'c'));
		pDelta->insert(std::string(100, 'z'));

		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 100 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Act:
		RemoveAllIdentifiersWithGroup(*pDelta, *pGroupedDelta, Height(3));

		// Assert:
		EXPECT_EQ(1u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(100));

		EXPECT_EQ(2u, pGroupedDelta->size());
		EXPECT_TRUE(pGroupedDelta->contains(Height(1)));
		EXPECT_TRUE(pGroupedDelta->contains(Height(7)));
	}

	TEST(TEST_CLASS, RemoveAllIdentifiersWithGroup_RemovesAllValuesInGroupAndIgnoresUnknownValues) {
		// Arrange:
		BaseSetType set;
		auto pDelta = set.rebase();
		pDelta->insert("a");
		pDelta->insert(std::string(9, 'c'));
		pDelta->insert(std::string(100, 'z'));

		HeightGroupedBaseSetType groupedSet;
		auto pGroupedDelta = groupedSet.rebase();
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(1)), { 100 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(3)), { 1, 4, 9 }));
		pGroupedDelta->insert(AddValues(TestIdentifierGroup(Height(7)), { 25, 26 }));

		// Act:
		RemoveAllIdentifiersWithGroup(*pDelta, *pGroupedDelta, Height(3));

		// Assert:
		EXPECT_EQ(1u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(100));

		EXPECT_EQ(2u, pGroupedDelta->size());
		EXPECT_TRUE(pGroupedDelta->contains(Height(1)));
		EXPECT_TRUE(pGroupedDelta->contains(Height(7)));
	}

	// endregion

}}
