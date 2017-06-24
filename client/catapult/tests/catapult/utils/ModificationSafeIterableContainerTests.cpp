#include "catapult/utils/ModificationSafeIterableContainer.h"
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"
#include <list>

namespace catapult { namespace utils {

	namespace {
		using IntIterableList = ModificationSafeIterableContainer<std::list<int>>;

		template<typename TIterator>
		std::vector<int> ToVector(TIterator begin, TIterator end) {
			std::vector<int> values;
			for (auto iter = begin; end != iter; ++iter)
				values.push_back(*iter);

			return values;
		}

		void PushAll(IntIterableList& container, const std::vector<int>& values) {
			for (auto value : values)
				container.push_back(value);
		}

		void AssertContents(IntIterableList& container, const std::vector<int>& expectedValues) {
			// Assert:
			EXPECT_EQ(expectedValues.empty(), container.empty());
			EXPECT_EQ(expectedValues.size(), container.size());
			EXPECT_EQ(expectedValues, ToVector(container.begin(), container.end()));
			EXPECT_EQ(expectedValues, ToVector(container.cbegin(), container.cend()));
		}
	}

	TEST(ModificationSafeIterableContainerTests, CanCreateEmptyContainer) {
		// Act:
		IntIterableList container;

		// Assert:
		AssertContents(container, {});
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyForEmptyContainer) {
		// Act:
		IntIterableList container;

		// Assert:
		EXPECT_FALSE(!!container.next());
		EXPECT_FALSE(!!container.next());
		EXPECT_FALSE(!!container.next());
	}

	TEST(ModificationSafeIterableContainerTests, CanCreateSingleElementContainer) {
		// Act:
		IntIterableList container;
		container.push_back(7);

		// Assert:
		AssertContents(container, { 7 });
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyForSingleElementContainer) {
		// Act:
		IntIterableList container;
		container.push_back(7);

		// Assert:
		EXPECT_EQ(7, *container.next());
		EXPECT_EQ(7, *container.next());
		EXPECT_EQ(7, *container.next());
	}

	TEST(ModificationSafeIterableContainerTests, CanCreateMultiElementContainer) {
		// Act:
		IntIterableList container;
		container.push_back(1);
		container.push_back(5);
		container.push_back(4);

		// Assert:
		AssertContents(container, { 1, 5, 4 });
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyForMultiElementContainer) {
		// Act:
		IntIterableList container;
		container.push_back(1);
		container.push_back(5);
		container.push_back(4);

		// Assert:
		EXPECT_EQ(1, *container.next());
		EXPECT_EQ(5, *container.next());
		EXPECT_EQ(4, *container.next());
		EXPECT_EQ(1, *container.next());
	}

	TEST(ModificationSafeIterableContainerTests, CanClearContainer) {
		// Arrange:
		IntIterableList container;
		PushAll(container, { 5, 7, 3, 2 });

		// Act:
		container.clear();

		// Assert:
		AssertContents(container, {});
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyAfterClear) {
		// Arrange:
		IntIterableList container;
		PushAll(container, { 5, 7, 3, 2 });

		// Sanity:
		EXPECT_EQ(5, *container.next());
		EXPECT_EQ(7, *container.next());

		// Act:
		container.clear();

		// Assert:
		EXPECT_FALSE(!!container.next());
		EXPECT_FALSE(!!container.next());
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyAfterInsertAfterClear) {
		// Arrange:
		IntIterableList container;
		PushAll(container, { 5, 7, 3, 2 });

		// Sanity:
		EXPECT_EQ(5, *container.next());
		EXPECT_EQ(7, *container.next());

		// Act:
		container.clear();
		container.push_back(4);

		// Assert:
		EXPECT_EQ(4, *container.next());
		EXPECT_EQ(4, *container.next());
	}

	namespace {
		void RunEraseTest(
				const std::function<IntIterableList::iterator (IntIterableList&)>& getIterator,
				const std::vector<int>& expectedContents) {
			// Arrange:
			IntIterableList container;
			PushAll(container, { 5, 7, 3, 2 });

			// Act:
			container.erase(getIterator(container));

			// Assert:
			AssertContents(container, expectedContents);
		}
	}

	TEST(ModificationSafeIterableContainerTests, CanEraseFirstElement) {
		// Assert:
		RunEraseTest([](auto& container) { return container.begin(); }, { 7, 3, 2 });
	}

	TEST(ModificationSafeIterableContainerTests, CanEraseMiddleElement) {
		// Assert:
		RunEraseTest([](auto& container) { return ++container.begin(); }, { 5, 3, 2 });
	}

	TEST(ModificationSafeIterableContainerTests, CanEraseLastElement) {
		// Assert:
		RunEraseTest([](auto& container) { return --container.end(); }, { 5, 7, 3 });
	}

	namespace {
		void RunNextAfterEraseTest(
				const std::function<IntIterableList::iterator (IntIterableList&)>& getIterator,
				const std::vector<int>& expectedBeforeValues,
				const std::vector<int>& expectedAfterValues) {
			CATAPULT_LOG(debug) << "before values = " << expectedBeforeValues.size()
					<< ", after values = " << expectedAfterValues.size();

			// Arrange:
			IntIterableList container;
			PushAll(container, { 5, 7, 3, 2 });

			size_t i = 0;
			for (auto value : expectedBeforeValues)
				EXPECT_EQ(value, *container.next()) << "[before] next " << i;

			// Act:
			container.erase(getIterator(container));

			// Assert:
			for (auto value : expectedAfterValues)
				EXPECT_EQ(value, *container.next()) << "[after] next " << i;
		}
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyAfterEraseFirstElement) {
		// Assert:
		auto getIterator = [](auto& container) { return container.begin(); };
		RunNextAfterEraseTest(getIterator, {}, { 7, 3, 2, 7 });
		RunNextAfterEraseTest(getIterator, { 5 }, { 7, 3, 2, 7 });
		RunNextAfterEraseTest(getIterator, { 5, 7 }, { 3, 2, 7 });
		RunNextAfterEraseTest(getIterator, { 5, 7, 3 }, { 2, 7 });
		RunNextAfterEraseTest(getIterator, { 5, 7, 3, 2 }, { 7 });
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyAfterEraseMiddleElement) {
		// Assert:
		auto getIterator = [](auto& container) { return ++container.begin(); };
		RunNextAfterEraseTest(getIterator, {}, { 5, 3, 2, 5, 3 });
		RunNextAfterEraseTest(getIterator, { 5 }, { 3, 2, 5, 3 });
		RunNextAfterEraseTest(getIterator, { 5, 7 }, { 3, 2, 5, 3 });
		RunNextAfterEraseTest(getIterator, { 5, 7, 3 }, { 2, 5, 3 });
		RunNextAfterEraseTest(getIterator, { 5, 7, 3, 2 }, { 5, 3 });
	}

	TEST(ModificationSafeIterableContainerTests, NextBehavesCorrectlyAfterEraseLastElement) {
		// Assert:
		auto getIterator = [](auto& container) { return --container.end(); };
		RunNextAfterEraseTest(getIterator, {}, { 5, 7, 3, 5 });
		RunNextAfterEraseTest(getIterator, { 5 }, { 7, 3, 5 });
		RunNextAfterEraseTest(getIterator, { 5, 7 }, { 3, 5 });
		RunNextAfterEraseTest(getIterator, { 5, 7, 3 }, { 5 });
		RunNextAfterEraseTest(getIterator, { 5, 7, 3, 2 }, { 5, 7, 3, 5 });
	}

	TEST(ModificationSafeIterableContainerTests, NextIfReturnsNullptrIfContainerIsEmpty) {
		// Arrange:
		IntIterableList container;

		// Act:
		auto pValue = container.nextIf([](const auto&) { return true; });

		// Assert:
		EXPECT_FALSE(!!pValue);
	}

	TEST(ModificationSafeIterableContainerTests, NextIfReturnsFirstMatchingElement) {
		// Arrange:
		IntIterableList container;
		PushAll(container, { 5, 4, 3, 2 });

		// Act:
		auto pValue = container.nextIf([](auto value) { return 0 == value % 2; });

		// Assert:
		ASSERT_TRUE(!!pValue);
		EXPECT_EQ(4, *pValue);
	}

	TEST(ModificationSafeIterableContainerTests, NextIfReturnsFirstMatchingElementRelativeToNextPosition) {
		// Arrange:
		IntIterableList container;
		PushAll(container, { 5, 4, 3, 2 });
		container.next(); // 5
		container.next(); // 4

		// Act:
		auto pValue = container.nextIf([](auto value) { return 0 == value % 2; });

		// Assert:
		ASSERT_TRUE(!!pValue);
		EXPECT_EQ(2, *pValue);
	}

	TEST(ModificationSafeIterableContainerTests, NextIfReturnsNullptrIfNoElementsMatch) {
		// Arrange:
		IntIterableList container;
		PushAll(container, { 5, 4, 3, 2 });

		// Act:
		auto pValue = container.nextIf([](auto value) { return 0 == value % 7; });

		// Assert:
		EXPECT_FALSE(!!pValue);
	}
}}
