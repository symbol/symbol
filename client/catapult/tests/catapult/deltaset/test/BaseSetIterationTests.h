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

#pragma once
#include "BaseSetTestsInclude.h"
#include "catapult/deltaset/BaseSetDeltaIterationView.h"
#include "catapult/deltaset/BaseSetIterationView.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region BaseSetIterationCommonTests

	/// All iteration tests that apply to both BaseSet and BaseSetDelta.
	template<typename TTraits>
	class BaseSetIterationCommonTests {
	private:
		using SetType = decltype(*TTraits::Create());
		using IterableSetType = decltype(MakeIterableView(*TTraits::Create()));

	protected:
		static void AssertBeginEndIteratorConsistency(const SetType& set) {
			// Sanity: ensure begin and end iterators are only equal when the set is empty
			auto iterableSet = MakeIterableView(set);
			if (set.empty())
				EXPECT_EQ(iterableSet.begin(), iterableSet.end());
			else
				EXPECT_NE(iterableSet.begin(), iterableSet.end());
		}

		static std::set<TestElement> ExtractElements(const SetType& set) {
			// Act: iterate over the set and extract all elements
			std::set<TestElement> elements;
			size_t numIteratedElements = 0;

			for (const auto& element : MakeIterableView(set)) {
				elements.insert(*TTraits::ToPointerFromStorage(element));
				++numIteratedElements;
			}

			// Sanity: iterators are consistent with size and there are no duplicate elements in the set
			AssertBeginEndIteratorConsistency(set);
			EXPECT_EQ(set.size(), numIteratedElements);
			return elements;
		}

	private:
		static void AssertIteration(const SetType& set, const std::set<TestElement>& expectedElements) {
			// Act:
			auto actualElements = ExtractElements(set);

			// Assert:
			EXPECT_EQ(expectedElements, actualElements);

			// Sanity: the iterator elements are const
			AssertConstIterator(set);
		}

	public:
		static void AssertCanIterateThroughEmptySetWithConstIterator() {
			// Assert:
			AssertIteration(*TTraits::Create(), {});
		}

		static void AssertCanIterateThroughSingleValueSetWithConstIterator() {
			// Assert:
			AssertIteration(*TTraits::CreateWithElements(1), { TestElement("TestElement", 0) });
		}

		static void AssertCanIterateThroughMultiValueSetWithConstIterator() {
			// Assert:
			std::set<TestElement> expectedElements{
				TestElement("TestElement", 0),
				TestElement("TestElement", 1),
				TestElement("TestElement", 2)
			};
			AssertIteration(*TTraits::CreateWithElements(3), expectedElements);
		}
	};

#define MAKE_COMMON_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BaseSetIterationCommonTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_COMMON_BASE_SET_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	MAKE_COMMON_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, CanIterateThroughEmptySetWithConstIterator) \
	MAKE_COMMON_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, CanIterateThroughSingleValueSetWithConstIterator) \
	MAKE_COMMON_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, CanIterateThroughMultiValueSetWithConstIterator)

	// endregion

	// region BaseSetIterationTests

	/// All iteration tests that apply to BaseSet but not BaseSetDelta.
	/// \note Iteration is an extended feature and can be disabled from testing by undefining DEFINE_BASE_SET_ITERATION_TESTS.
	template<typename TTraits>
	class BaseSetIterationTests {
	public:
		static void AssertFindIteratorReturnsIteratorToElementWhenElementExists() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto iterableSet = MakeIterableView(*pSet);
			auto element = TTraits::CreateElement("TestElement", 1);

			// Act:
			auto iter = iterableSet.findIterator(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(*TTraits::ToPointer(element), *TTraits::ToPointerFromStorage(*iter));
		}

		static void AssertFindIteratorReturnsCendWhenElementDoesNotExist() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto iterableSet = MakeIterableView(*pSet);
			auto element = TTraits::CreateElement("TestElement", 4);

			// Act:
			auto iter = iterableSet.findIterator(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(iterableSet.end(), iter);
		}
	};

#define MAKE_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BaseSetIterationTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_BASE_SET_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_COMMON_BASE_SET_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, FindIteratorReturnsIteratorToElementWhenElementExists) \
	MAKE_BASE_SET_ITERATION_TEST(TEST_CLASS, TRAITS, FindIteratorReturnsCendWhenElementDoesNotExist)

	// endregion

	// region BaseSetIterationDeltaTests

	/// All iteration tests that apply to BaseSetDelta but not BaseSet.
	/// \note Iteration is an extended feature and can be disabled from testing by undefining DEFINE_BASE_SET_DELTA_ITERATION_TESTS.
	template<typename TTraits>
	class BaseSetIterationDeltaTests
			: private BaseSetDeltaTestUtils<TTraits>
			, private BaseSetIterationCommonTests<TTraits> {
	private:
		using BaseSetDeltaTestUtils<TTraits>::CreateSetForBatchFindTests;
		using BaseSetDeltaTestUtils<TTraits>::CreateExpectedElementsForBatchFindTests;
		using BaseSetDeltaTestUtils<TTraits>::SetDummyValue;
		using BaseSetIterationCommonTests<TTraits>::AssertBeginEndIteratorConsistency;
		using BaseSetIterationCommonTests<TTraits>::ExtractElements;

	private:
		using SetType = decltype(*TTraits::Create());
		using IteratorType = decltype(MakeIterableView(*TTraits::Create()).begin());

	public:
		static void AssertDeltaIterationIncludesOriginalElements() {
			// Arrange: commit so that all elements become original elements
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();

			// Act:
			auto actualElements = ExtractElements(*pDelta);

			// Assert:
			std::set<TestElement> expectedElements{
				TestElement("TestElement", 0),
				TestElement("TestElement", 1),
				TestElement("TestElement", 2)
			};
			EXPECT_EQ(expectedElements, actualElements);
			AssertDeltaSizes(pDelta, 3, 0, 0, 0);
		}

		static void AssertDeltaIterationCanIterateOnlyInsertedElements() {
			// Arrange:
			auto pDelta = TTraits::Create();
			pDelta->emplace("TestElement", static_cast<unsigned int>(7));
			pDelta->emplace("TestElement", static_cast<unsigned int>(4));

			// Act:
			auto actualElements = ExtractElements(*pDelta);

			// Assert:
			std::set<TestElement> expectedElements{
				TestElement("TestElement", 4),
				TestElement("TestElement", 7)
			};
			EXPECT_EQ(expectedElements, actualElements);
			AssertDeltaSizes(pDelta, 0, 2, 0, 0);
		}

		static void AssertDeltaIterationIncludesInsertedElements() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			pDelta->emplace("TestElement", static_cast<unsigned int>(7));
			pDelta->emplace("TestElement", static_cast<unsigned int>(4));

			// Act:
			auto actualElements = ExtractElements(*pDelta);

			// Assert:
			std::set<TestElement> expectedElements{
				TestElement("TestElement", 0),
				TestElement("TestElement", 1),
				TestElement("TestElement", 2),
				TestElement("TestElement", 4),
				TestElement("TestElement", 7)
			};
			EXPECT_EQ(expectedElements, actualElements);
			AssertDeltaSizes(pDelta, 3, 2, 0, 0);
		}

		static void AssertDeltaIterationExcludesRemovedElements() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			pDelta->emplace("TestElement", static_cast<unsigned int>(7));
			pDelta->emplace("TestElement", static_cast<unsigned int>(4));
			pDelta->remove(TTraits::CreateKey("TestElement", 1));
			pDelta->remove(TTraits::CreateKey("TestElement", 4));

			// Act:
			auto actualElements = ExtractElements(*pDelta);

			// Assert:
			std::set<TestElement> expectedElements{
				TestElement("TestElement", 0),
				TestElement("TestElement", 2),
				TestElement("TestElement", 7)
			};
			EXPECT_EQ(expectedElements, actualElements);
			AssertDeltaSizes(pDelta, 3, 2, 2, 0);
		}

	private:
		static std::set<size_t> ExtractDummyValues(const SetType& set) {
			std::set<size_t> dummyValues;
			size_t numIteratedElements = 0;
			for (const auto& element : MakeIterableView(set)) {
				dummyValues.insert(TTraits::ToPointerFromStorage(element)->Dummy);
				++numIteratedElements;
			}

			// Sanity: iterators are consistent with size and there are no duplicate elements in the set
			AssertBeginEndIteratorConsistency(set);
			EXPECT_EQ(set.size(), numIteratedElements);
			return dummyValues;
		}

	public:
		static void AssertDeltaIterationIncludesCopiedElements() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			SetDummyValue(*pDelta, 0, 0);
			SetDummyValue(*pDelta, 1, 1);
			SetDummyValue(*pDelta, 2, 2);
			pDelta.commit(); // the 3 elements are now original elements with dummy values set

			// - change the dummy value for one element
			SetDummyValue(*pDelta, 1, 42);

			// Act:
			auto actualDummyValues = ExtractDummyValues(*pDelta);

			// Assert: iterating should pick up the new dummy value
			//         (in the case of mutable, the copied element will be returned in place of the original element)
			//         (in the case of immutable, the original element with the changed value will be returned)
			std::set<size_t> expectedDummyValues{ 0, 42, 2 };
			EXPECT_EQ(expectedDummyValues, actualDummyValues);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}

		static void AssertDeltaIterationCanIterateOnlyCopiedElements() {
			// Arrange:
			constexpr auto Num_Elements = 3u;
			auto pDelta = TTraits::CreateWithElements(Num_Elements);
			for (auto i = 0u; i < Num_Elements; ++i)
				SetDummyValue(*pDelta, i, i);

			pDelta.commit(); // the 3 elements are now original elements with dummy values set

			// - make copies of all the elements
			for (auto i = 0u; i < Num_Elements; ++i)
				SetDummyValue(*pDelta, i, i + 7);

			// Act:
			auto actualDummyValues = ExtractDummyValues(*pDelta);

			// Assert: iterating should pick up the new dummy values
			std::set<size_t> expectedDummyValues{ 7, 8, 9 };
			EXPECT_EQ(expectedDummyValues, actualDummyValues);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 3 : 0);
		}

		static void AssertDeltaIterationDoesNotReiterateNewlyCopiedElements() {
			// Arrange: create a set with three original elements with dummy values
			auto pDelta = TTraits::CreateWithElements(3);
			SetDummyValue(*pDelta, 0, 0);
			SetDummyValue(*pDelta, 1, 1);
			SetDummyValue(*pDelta, 2, 2);
			pDelta.commit(); // the 3 elements are now original elements with dummy values set

			// - add a fourth (uncommitted) element with a dummy value
			pDelta->emplace("TestElement", static_cast<unsigned int>(3));
			SetDummyValue(*pDelta, 3, 3);

			// Act: iterate
			std::set<size_t> dummyValues;
			size_t numIteratedElements = 0;
			for (const auto& element : MakeIterableView(*pDelta)) {
				auto pCurrentElement = TTraits::ToPointerFromStorage(element);
				auto dummyValue = pCurrentElement->Dummy;
				dummyValues.insert(dummyValue);
				++numIteratedElements;

				// - change the dummy value of the middle elements
				if (1 > dummyValue || dummyValue > 2)
					continue;

				auto pElementCopy = pDelta->find(TTraits::CreateKey(pCurrentElement->Name, pCurrentElement->Value)).get();
				pElementCopy->Dummy = dummyValue + 40;
			}

			// Assert: initial iteration should pick up the old dummy values
			std::set<size_t> expectedDummyValues{ 0, 1, 2, 3 };
			EXPECT_EQ(expectedDummyValues.size(), numIteratedElements);
			EXPECT_EQ(expectedDummyValues, dummyValues);

			// Sanity: reiterating should pick up the new dummy values
			expectedDummyValues = { 0, 41, 42, 3 };
			EXPECT_EQ(expectedDummyValues, ExtractDummyValues(*pDelta));
			AssertDeltaSizes(pDelta, 3, 1, 0, TTraits::IsElementMutable() ? 2 : 0);
		}

		static void AssertDeltaCannotDereferenceAtEnd() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto iterableDelta = MakeIterableView(*pDelta);
			auto iter = iterableDelta.end();

			// Act + Assert:
			EXPECT_THROW(*iter, catapult_out_of_range);
			EXPECT_THROW(iter.operator->(), catapult_out_of_range);
		}

		static void AssertDeltaCannotAdvancePrefixIteratorBeyondEnd() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto iterableDelta = MakeIterableView(*pDelta);
			auto iter = iterableDelta.end();

			// Act + Assert:
			EXPECT_THROW(++iter, catapult_out_of_range);
			EXPECT_THROW(++iter, catapult_out_of_range);
		}

		static void AssertDeltaCannotAdvancePostfixIteratorBeyondEnd() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto iterableDelta = MakeIterableView(*pDelta);
			auto iter = iterableDelta.end();

			// Act + Assert:
			EXPECT_THROW(iter++, catapult_out_of_range);
			EXPECT_THROW(iter++, catapult_out_of_range);
		}

		static void AssertDeltaBeginEndIteratorsBasedOnSameContainerAreEqual() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto iterableDelta = MakeIterableView(*pDelta);

			// Act + Assert:
			EXPECT_EQ(iterableDelta.begin(), iterableDelta.begin());
			EXPECT_EQ(iterableDelta.end(), iterableDelta.end());
			EXPECT_NE(iterableDelta.begin(), iterableDelta.end());
		}

		static void AssertDeltaBeginEndIteratorsBasedOnDifferentContainerAreNotEqual() {
			// Arrange:
			auto pDelta1 = TTraits::CreateWithElements(3);
			auto iterableDelta1 = MakeIterableView(*pDelta1);

			auto pDelta2 = TTraits::CreateWithElements(3);
			auto iterableDelta2 = MakeIterableView(*pDelta2);

			// Act + Assert:
			EXPECT_NE(iterableDelta1.begin(), iterableDelta2.begin());
			EXPECT_NE(iterableDelta1.end(), iterableDelta2.end());
		}

	private:
		static void AssertDeltaIteration(const consumer<IteratorType&>& increment) {
			// Arrange:
			auto pDelta = CreateSetForBatchFindTests();
			auto iterableDelta = MakeIterableView(*pDelta);

			// Act:
			std::set<TestElement> actualElements;
			size_t numIteratedElements = 0;
			for (auto iter = iterableDelta.begin(); iterableDelta.end() != iter; increment(iter)) {
				actualElements.insert(*TTraits::ToPointerFromStorage(*iter));
				++numIteratedElements;

				// Sanity: * and -> return same element
				EXPECT_EQ(TTraits::ToPointerFromStorage(*iter), TTraits::ToPointerFromStorage(*(iter.operator->())));
			}

			// Assert:
			EXPECT_EQ(pDelta->size(), numIteratedElements);
			auto expectedElements = CreateExpectedElementsForBatchFindTests();
			EXPECT_EQ(expectedElements, actualElements);

			// Sanity: value_type should be const
			EXPECT_TRUE(std::is_const<typename decltype(iterableDelta.begin())::value_type>());
			AssertDeltaSizes(pDelta, 4, 3, 2, TTraits::IsElementMutable() ? 1 : 0);
		}

	public:
		static void AssertDeltaIterationSupportsPostfixIterator() {
			// Assert:
			AssertDeltaIteration([](auto& iter) { iter++; });
		}

		static void AssertDeltaIterationSupportsPrefixIterator() {
			// Assert:
			AssertDeltaIteration([](auto& iter) { ++iter; });
		}
	};

#define MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BaseSetIterationDeltaTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_BASE_SET_DELTA_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_COMMON_BASE_SET_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationIncludesOriginalElements) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationCanIterateOnlyInsertedElements) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationIncludesInsertedElements) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationExcludesRemovedElements) \
	\
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationIncludesCopiedElements) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationCanIterateOnlyCopiedElements) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationDoesNotReiterateNewlyCopiedElements) \
	\
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaCannotDereferenceAtEnd) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaCannotAdvancePrefixIteratorBeyondEnd) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaCannotAdvancePostfixIteratorBeyondEnd) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaBeginEndIteratorsBasedOnSameContainerAreEqual) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaBeginEndIteratorsBasedOnDifferentContainerAreNotEqual) \
	\
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationSupportsPostfixIterator) \
	MAKE_BASE_SET_DELTA_ITERATION_TEST(TEST_CLASS, TRAITS, DeltaIterationSupportsPrefixIterator) \

	// endregion
}}
