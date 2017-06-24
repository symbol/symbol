#include "catapult/utils/WeakContainer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	TEST(WeakContainerTests, ContainerIsInitiallyEmpty) {
		// Act:
		WeakContainer<int> container;

		// Assert:
		EXPECT_EQ(0u, container.size());
	}

	namespace {
		std::vector<std::shared_ptr<int>> GenerateVector(const std::vector<int>& values) {
			std::vector<std::shared_ptr<int>> pointers;
			for (auto value : values)
				pointers.push_back(std::make_shared<int>(value));

			return pointers;
		}

		void InitializeFromVector(WeakContainer<int>& container, const std::vector<std::shared_ptr<int>>& pointers) {
			for (const auto& pointer : pointers)
				container.insert(pointer);
		}
	}

	TEST(WeakContainerTests, CanAddItemsToContainer) {
		// Arrange:
		auto vec = GenerateVector({ 7, 3, 8 });

		// Act:
		WeakContainer<int> container;
		InitializeFromVector(container, vec);

		// Assert:
		EXPECT_EQ(3u, container.size());
	}

	TEST(WeakContainerTests, AlreadyDestroyedItemsAreRemoveByInsert) {
		// Arrange:
		auto vec = GenerateVector({ 7, 3, 8, 5 });
		WeakContainer<int> container;
		InitializeFromVector(container, vec);

		// - destroy three elements in vec
		auto pInt = std::make_shared<int>(11);
		vec[0].reset();
		vec[2].reset();
		vec[3].reset();

		// Act: add a fourth element to container
		container.insert(pInt);

		// Assert: only 2 elements remain in the container v[1] and pInt
		EXPECT_EQ(2u, container.size());
	}

	TEST(WeakContainerTests, AlreadyDestroyedItemsAreRemoveBySize) {
		// Arrange:
		auto vec = GenerateVector({ 7, 3, 8, 5 });
		WeakContainer<int> container;
		InitializeFromVector(container, vec);

		// - destroy three elements in vec
		auto pInt = std::make_shared<int>(11);
		vec[0].reset();
		vec[2].reset();
		vec[3].reset();

		// Act + Assert: the elements should be pruned immediately
		EXPECT_EQ(1u, container.size());
	}

	TEST(WeakContainerTests, ClearRemovesAllItems) {
		// Arrange: create a container around 2 valid and 1 invalid element
		auto vec = GenerateVector({ 7, 3, 8 });
		WeakContainer<int> container;
		InitializeFromVector(container, vec);
		vec[1].reset();

		// Act:
		container.clear();

		// Assert: both valid and invalid pointers were removed
		EXPECT_EQ(0u, container.size());
	}

	TEST(WeakContainerTests, ClearCallsCustomCloseFunctionForAllValidItems) {
		// Arrange: create a container around 2 valid and 1 invalid element
		auto vec = GenerateVector({ 7, 3, 8 });
		std::vector<int> closeIds;
		WeakContainer<int> container([&closeIds](const auto& id) { closeIds.push_back(id); });
		InitializeFromVector(container, vec);
		vec[1].reset();

		// Act:
		container.clear();

		// Assert: both valid and invalid pointers were removed but close was only called for valid ones
		std::vector<int> expectedCloseIds{ 7, 8 };
		EXPECT_EQ(0u, container.size());
		EXPECT_EQ(expectedCloseIds, closeIds);
	}
}}
