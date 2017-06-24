#include "src/state/GroupedMosaicIds.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	namespace {
		struct TestKey_tag {};
		using TestKey = utils::BaseValue<uint64_t, TestKey_tag>;

		using TestKeyGroupedMosaicIds = GroupedMosaicIds<TestKey>;

		void AssertMosaicIds(
				const TestKeyGroupedMosaicIds::MosaicIds& mosaicIds,
				const std::vector<MosaicId::ValueType>& expectedIds) {
			ASSERT_EQ(expectedIds.size(), mosaicIds.size());

			for (auto id : expectedIds)
				EXPECT_EQ(1u, mosaicIds.count(MosaicId(id))) << "mosaic id " << id;
		}
	}

	// region ctor

	TEST(GroupedMosaicIdsTests, CanCreateEmptyContainer) {
		// Act:
		TestKeyGroupedMosaicIds container(TestKey(123));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_TRUE(container.empty());
		EXPECT_EQ(0u, container.size());
		AssertMosaicIds(container.mosaicIds(), {});
	}

	// endregion

	// region add

	TEST(GroupedMosaicIdsTests, CanAddSingleMosaicId) {
		// Arrange:
		TestKeyGroupedMosaicIds container(TestKey(123));

		// Act:
		container.add(MosaicId(234));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(1u, container.size());
		AssertMosaicIds(container.mosaicIds(), { 234 });
	}

	TEST(GroupedMosaicIdsTests, CanAddMultipleMosaicIds) {
		// Arrange:
		TestKeyGroupedMosaicIds container(TestKey(123));
		std::vector<MosaicId::ValueType> expectedIds{ 135, 246, 357 };

		// Act:
		for (auto id : expectedIds)
			container.add(MosaicId(id));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(3u, container.size());
		AssertMosaicIds(container.mosaicIds(), expectedIds);
	}

	// endregion

	// region remove

	namespace {
		auto CreateTestKeyGroupedMosaicIds(TestKey testKey, const std::vector<MosaicId::ValueType>& ids) {
			TestKeyGroupedMosaicIds container(testKey);
			for (auto id : ids)
				container.add(MosaicId(id));

			// Sanity:
			EXPECT_EQ(testKey, container.key());
			EXPECT_FALSE(container.empty());
			EXPECT_EQ(ids.size(), container.size());
			AssertMosaicIds(container.mosaicIds(), ids);
			return container;
		}
	}

	TEST(GroupedMosaicIdsTests, RemoveUnknownMosaicIdIsNoOp) {
		// Arrange:
		auto container = CreateTestKeyGroupedMosaicIds(TestKey(123), { 234, 345, 456 });

		// Act:
		container.remove(MosaicId(678));
		container.remove(MosaicId(789));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(3u, container.size());
		AssertMosaicIds(container.mosaicIds(), { 234, 345, 456 });
	}

	TEST(GroupedMosaicIdsTests, CanRemoveSingleMosaicId) {
		// Arrange:
		auto container = CreateTestKeyGroupedMosaicIds(TestKey(123), { 234, 345, 456 });

		// Act:
		container.remove(MosaicId(345));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());
		AssertMosaicIds(container.mosaicIds(), { 234, 456 });
	}

	TEST(GroupedMosaicIdsTests, CanRemoveMultipleMosaicIds) {
		// Arrange:
		auto container = CreateTestKeyGroupedMosaicIds(TestKey(123), { 234, 345, 456, 567, 678 });

		// Act:
		container.remove(MosaicId(345));
		container.remove(MosaicId(456));
		container.remove(MosaicId(678));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());
		AssertMosaicIds(container.mosaicIds(), { 234, 567 });
	}

	TEST(GroupedMosaicIdsTests, CanRemoveAllMosaicIds) {
		// Arrange:
		auto container = CreateTestKeyGroupedMosaicIds(TestKey(123), { 234, 345, 456 });

		// Act:
		container.remove(MosaicId(234));
		container.remove(MosaicId(345));
		container.remove(MosaicId(456));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_TRUE(container.empty());
		EXPECT_EQ(0u, container.size());
		AssertMosaicIds(container.mosaicIds(), {});
	}

	// endregion
}}
