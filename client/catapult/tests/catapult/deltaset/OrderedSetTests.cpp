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

#include "catapult/deltaset/OrderedSet.h"
#include "tests/catapult/deltaset/test/BaseSetDeltaTests.h"
#include "tests/catapult/deltaset/test/BaseSetTests.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS OrderedSetTests

	namespace {
		template<typename TSet>
		using PruningBoundaryType = PruningBoundary<std::remove_const_t<typename TSet::ElementType>>;

		template<typename TSet>
		void CommitWithoutPruning(TSet& set) {
			set.commit(PruningBoundaryType<TSet>());
		}

		template<typename TSet, typename TElement>
		void CommitWithPruning(TSet& set, const TElement& element) {
			set.commit(PruningBoundaryType<TSet>(element));
		}

		template<typename TElementTraits>
		struct OrderedSetTraits {
			using Type = OrderedSet<TElementTraits>;
			using DeltaType = OrderedSetDelta<TElementTraits>;
			using ElementType = typename TElementTraits::ElementType;
			using SetTraits = typename DeltaType::SetTraits;

			template<typename... TArgs>
			static auto Create(TArgs&&... args) {
				return std::make_shared<OrderedSet<TElementTraits>>(std::forward<TArgs>(args)...);
			}

			static void Commit(Type& set) {
				CommitWithoutPruning(set);
			}
		};

		template<typename TMutabilityTraits>
		using OrderedSetTypeTraits = OrderedSetTraits<TMutabilityTraits>;

		using OrderedSetMutableTraits = OrderedSetTypeTraits<MutableTypeTraits<test::MutableTestElement>>;
		using OrderedSetImmutableTraits = OrderedSetTypeTraits<ImmutableTypeTraits<const test::ImmutableTestElement>>;
	}

// base (mutable)
DEFINE_MUTABLE_BASE_SET_TESTS_FOR(OrderedSetMutable)

// base (immutable)
DEFINE_IMMUTABLE_BASE_SET_TESTS_FOR(OrderedSetImmutable)

#define MAKE_ORDERED_SET_TEST(TEST_NAME, TYPE) \
	TEST(BaseOrderedSet##TYPE##Tests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BaseTraits<deltaset::OrderedSet##TYPE##Traits>>(); \
	}

#define ORDERED_SET_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, Mutable) \
	MAKE_ORDERED_SET_TEST(TEST_NAME, Immutable) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region commit

	ORDERED_SET_TEST(CommitWithUnsetPruningBoundaryCommitsToOriginalElementsWithoutPruning) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto pDelta = pSet->rebase();
		pDelta->emplace("MyTestElement", static_cast<unsigned int>(123));
		pDelta->remove(TTraits::CreateElement("TestElement", 0));
		pDelta->remove(TTraits::CreateElement("TestElement", 2));

		// Act:
		CommitWithoutPruning(*pSet);

		// Assert:
		TTraits::AssertContents(*pSet, {
			TTraits::CreateElement("TestElement", 1),
			TTraits::CreateElement("MyTestElement", 123)
		});
	}

	ORDERED_SET_TEST(CommitPrunesElementsPreviousToPruningBoundary) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();

		auto pruningBoundaryElement = TTraits::CreateElement("TestElement", 3);

		// Act:
		CommitWithPruning(*pSet, pruningBoundaryElement);

		// Assert:
		TTraits::AssertContents(*pSet, {
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4)
		});
	}

	ORDERED_SET_TEST(CommitIsNullOperationWhenPruningBoundaryIsEqualToFirstSetElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		pDelta->remove(TTraits::CreateElement("TestElement", 0));
		pDelta->remove(TTraits::CreateElement("TestElement", 1));
		CommitWithoutPruning(*pSet);

		auto pruningBoundaryElement = TTraits::CreateElement("TestElement", 2);

		// Sanity:
		auto firstElement = *MakeIterableView(*pSet).begin();
		EXPECT_EQ(*TTraits::ToPointer(firstElement), *TTraits::ToPointer(pruningBoundaryElement));

		// Act:
		CommitWithPruning(*pSet, pruningBoundaryElement);

		// Assert:
		TTraits::AssertContents(*pSet, {
			TTraits::CreateElement("TestElement", 2),
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4)
		});
	}

	ORDERED_SET_TEST(CommitIsNullOperationWhenPruningBoundaryIsSmallerThanFirstSetElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		pDelta->remove(TTraits::CreateElement("TestElement", 0));
		pDelta->remove(TTraits::CreateElement("TestElement", 1));
		CommitWithoutPruning(*pSet);

		auto pruningBoundaryElement = TTraits::CreateElement("TestElement", 1);

		// Sanity:
		auto firstElement = *MakeIterableView(*pSet).begin();
		EXPECT_TRUE(*TTraits::ToPointer(pruningBoundaryElement) < *TTraits::ToPointer(firstElement));

		// Act:
		CommitWithPruning(*pSet, pruningBoundaryElement);

		// Assert:
		TTraits::AssertContents(*pSet, {
			TTraits::CreateElement("TestElement", 2),
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4)
		});
	}

	ORDERED_SET_TEST(CommitPruningBoundaryDoesNotNeedToBeInSet) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();

		auto pruningBoundaryElement = TTraits::CreateElement("TestElement", 2);
		pDelta->remove(pruningBoundaryElement);
		CommitWithoutPruning(*pSet);

		// Sanity:
		EXPECT_FALSE(!!pSet->find(pruningBoundaryElement).get());

		// Act:
		CommitWithPruning(*pSet, pruningBoundaryElement);

		// Assert:
		TTraits::AssertContents(*pSet, {
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4)
		});
	}

	// endregion

	// region iteration

	ORDERED_SET_TEST(IterationWithReplacedElementPreservesOriginalOrder) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(6);
		auto pDelta = pSet->rebase();

		// - remove and reinsert the same element with a different dummy value
		{
			auto element = TTraits::CreateElement("TestElement", 3);
			element.Dummy = 123;
			pDelta->remove(element);
			pDelta->insert(element);
		}

		// Act: iterate and collect values
		std::vector<unsigned int> collectedValues;
		std::vector<size_t> collectedDummys;
		for (const auto& element : MakeIterableView(*pDelta)) {
			collectedValues.push_back(element.Value);
			collectedDummys.push_back(element.Dummy);
		}

		// Assert: iteration should be *ordered*
		EXPECT_EQ(std::vector<unsigned int>({ 0, 1, 2, 3, 4, 5 }), collectedValues);

		size_t expectedDummyValue = TTraits::IsElementMutable() ? 123 : 0;
		EXPECT_EQ(std::vector<size_t>({ 0, 0, 0, expectedDummyValue, 0, 0 }), collectedDummys);
	}

	// endregion
}}
