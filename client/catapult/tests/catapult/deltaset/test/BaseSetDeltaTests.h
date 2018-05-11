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

#pragma once
#include "BaseSetCommonTests.h"

namespace catapult { namespace test {

	/// All tests that apply to BaseSetDelta but not BaseSet.
	template<typename TTraits>
	class BaseSetDeltaTests : private BaseSetDeltaTestUtils<TTraits> {
	private:
		using BaseSetDeltaTestUtils<TTraits>::CreateSetForBatchFindTests;
		using BaseSetDeltaTestUtils<TTraits>::CreateExpectedElementsForBatchFindTests;
		using BaseSetDeltaTestUtils<TTraits>::SetDummyValue;

	public:
		// region empty

		static void AssertEmptyReturnsTrueIfAllOriginalElementsAreRemoved() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			for (auto i = 0u; i < 3; i++)
				pDelta->remove(TTraits::CreateKey("TestElement", i));

			// Assert:
			EXPECT_TRUE(pDelta->empty());
			AssertDeltaSizes(pDelta, 3, 0, 3, 0);
		}

		static void AssertEmptyReturnsFalseIfANonOriginalElementWasInserted() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			for (auto i = 0u; i < 3; i++)
				pDelta->remove(TTraits::CreateKey("TestElement", i));

			pDelta->insert(TTraits::CreateElement("TestElement", 123));

			// Assert:
			EXPECT_FALSE(pDelta->empty());
			AssertDeltaSizes(pDelta, 3, 1, 3, 0);
		}

		// endregion

		// region find

		static void AssertMutableBaseSetDeltaFindReturnsCopyForAnOriginalElement() {
			// Arrange:
			auto pSet = TTraits::CreateBase();
			auto pDelta = pSet->rebase();

			auto element = TTraits::CreateElement("TestElement", 4);
			pDelta->insert(element);
			pSet->commit();

			// Act:
			auto pBaseElement = pSet->find(TTraits::ToKey(element));
			auto pDeltaElement = pDelta->find(TTraits::ToKey(element));

			// Assert:
			EXPECT_NE(pBaseElement, pDeltaElement);
			AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 1);
		}

		static void AssertImmutableBaseSetDeltaFindReturnsOriginalElement() {
			// Arrange:
			auto pSet = TTraits::CreateBase();
			auto pDelta = pSet->rebase();

			auto element = TTraits::CreateElement("TestElement", 4);
			pDelta->insert(element);
			pSet->commit();

			// Act:
			auto pBaseElement = pSet->find(TTraits::ToKey(element));
			auto pDeltaElement = pDelta->find(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(pBaseElement, pDeltaElement);
			AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
		}

	private:
		// returns \c true if T is a shared_ptr, \c false otherwise
		template<typename T>
		static bool IsWrapped(T*) {
			return false;
		}

		template<typename T>
		static bool IsWrapped(const std::shared_ptr<T>&) {
			return true;
		}

	public:
		static void AssertMutableBaseSetDeltaFindReturnsNonConstCopy() {
			// Arrange:
			auto pSet = TTraits::CreateBase();
			auto pDelta = pSet->rebase();

			auto element = TTraits::CreateElement("TestElement", 4);
			pDelta->insert(element);
			pSet->commit();

			// Act:
			auto pElement1 = pDelta->find(TTraits::ToKey(element));
			pElement1->Dummy = 345;
			auto pElement2 = pDelta->find(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(345u, pElement2->Dummy);

			// - if the delta contains wrapped elements, they should be mutable
			// - if the delta contains raw elements, they should be const in sets (due to the underlying set storage
			//   since sets contain const elements to prevent changing keys) but mutable in maps
			// - Dummy being changed above is mutable
			if (IsWrapped(pDelta->find(TTraits::ToKey(element))) || AllowsNativeValueModification(*pSet))
				EXPECT_FALSE(std::is_const<decltype(Unwrap(pDelta->find(TTraits::ToKey(element))))>());
			else
				EXPECT_TRUE(std::is_const<decltype(Unwrap(pDelta->find(TTraits::ToKey(element))))>());

			// Sanity: the original (base) set always returns const elements
			EXPECT_TRUE(std::is_const<decltype(Unwrap(pSet->find(TTraits::ToKey(element))))>());
			AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 1);
		}

		static void AssertImmutableBaseSetDeltaFindReturnsConstCopy() {
			// Arrange:
			auto pSet = TTraits::CreateBase();
			auto pDelta = pSet->rebase();

			auto element = TTraits::CreateElement("TestElement", 4);
			pDelta->insert(element);
			pSet->commit();

			// Assert:
			EXPECT_TRUE(std::is_const<decltype(Unwrap(pDelta->find(TTraits::ToKey(element))))>());

			// Sanity: the original (base) set always returns const elements
			EXPECT_TRUE(std::is_const<decltype(Unwrap(pSet->find(TTraits::ToKey(element))))>());
			AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
		}

	private:
		template<typename T>
		static std::shared_ptr<const T> MakeConst(const std::shared_ptr<T>& pObject) {
			return pObject;
		}

	public:
		static void AssertBaseSetDeltaFindConstReturnsOriginalElement() {
			// Arrange:
			auto pSet = TTraits::CreateBase();
			auto pDelta = pSet->rebase();

			auto element = TTraits::CreateElement("TestElement", 4);
			pDelta->insert(element);
			pSet->commit();

			// Act:
			auto pBaseElement = pSet->find(TTraits::ToKey(element));
			auto pDeltaElement = MakeConst(pDelta)->find(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(pBaseElement, pDeltaElement);
			AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
		}

		static void AssertBaseSetDeltaFindConstReturnsConstCopy() {
			// Arrange:
			auto pSet = TTraits::CreateBase();
			auto pDelta = pSet->rebase();

			auto element = TTraits::CreateElement("TestElement", 4);
			pDelta->insert(element);
			pSet->commit();

			// Assert:
			EXPECT_TRUE(std::is_const<decltype(Unwrap(MakeConst(pDelta)->find(TTraits::ToKey(element))))>());
			AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
		}

	private:
		template<typename TSet>
		static void AssertBatchFind(TSet& set) {
			// Assert:
			auto expectedElements = CreateExpectedElementsForBatchFindTests();

			for (const auto& element : expectedElements)
				EXPECT_TRUE(!!set.find(TTraits::CreateKey(element.Name, element.Value))) << element;

			for (auto value : { 1u, 4u, 6u, 8u, 9u })
				EXPECT_FALSE(!!set.find(TTraits::CreateKey("TestElement", value))) << value;

			// Sanity:
			EXPECT_EQ(expectedElements.size(), set.size());
		}

	public:
		static void AssertBaseSetDeltaCanAccessAllElementsThroughFind() {
			// Arrange:
			auto pDelta = CreateSetForBatchFindTests();

			// Assert: note that (non-const) find will make copies of all elements
			AssertBatchFind<typename decltype(pDelta)::DeltaType>(*pDelta);
			AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 3 : 0);
		}

		static void AssertBaseSetDeltaCanAccessAllElementsThroughFindConst() {
			// Arrange:
			auto pDelta = CreateSetForBatchFindTests();

			// Assert:
			AssertBatchFind<const typename decltype(pDelta)::DeltaType>(*pDelta);
			AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 1 : 0);
		}

		// endregion

		// region insert

	private:
		static deltaset::InsertResult GetUpdatedResult() {
			// mutable elements can be updated, immutable ones cannot by definition
			return TTraits::IsElementMutable() ? deltaset::InsertResult::Updated : deltaset::InsertResult::Redundant;
		}

	public:
		static void AssertCanInsertElement() {
			// Arrange:
			auto pDelta = TTraits::Create();
			auto element = TTraits::CreateElement("MyTestElement", 321);
			auto expectedElements = typename TTraits::ElementVector{ element };

			// Act:
			auto result = pDelta->insert(element);

			// Assert:
			EXPECT_EQ(deltaset::InsertResult::Inserted, result);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 0, 1, 0, 0);
		}

		static void AssertCanInsertWithSuppliedParameters() {
			// Arrange:
			auto pDelta = TTraits::Create();
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("MyTestElement", 0),
				TTraits::CreateElement("AnotherTestElement", 123)
			};

			// Act:
			auto result1 = pDelta->emplace("MyTestElement");
			auto result2 = pDelta->emplace("AnotherTestElement", static_cast<unsigned int>(123));

			// Assert:
			EXPECT_EQ(deltaset::InsertResult::Inserted, result1);
			EXPECT_EQ(deltaset::InsertResult::Inserted, result2);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 0, 2, 0, 0);
		}

	private:
		static void AssertCopyInsertNewElement(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::Create();
			auto element = TTraits::CreateElement("TestElement", 4);
			TTraits::ToPointer(element)->Dummy = 123;
			auto initialResult = pDelta->insert(element); // element is not an original element

			// Sanity:
			EXPECT_EQ(deltaset::InsertResult::Inserted, initialResult);

			// Act:
			auto identicalElement = TTraits::CreateElement("TestElement", 4);
			std::vector<deltaset::InsertResult> results;
			for (auto i = 1u; i < 10; ++i) {
				TTraits::ToPointer(identicalElement)->Dummy = i;
				results.push_back(pDelta->insert(identicalElement));
			}

			auto pCurrent = pDelta->find(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(std::vector<deltaset::InsertResult>(results.size(), GetUpdatedResult()), results);

			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 0, 1, 0, 0);
		}

	public:
		static void AssertMutableBaseSetDeltaInsertUpdatesKnownElement() {
			// Assert:
			AssertCopyInsertNewElement(9);
		}

		static void AssertImmutableBaseSetDeltaInsertDoesNotUpdateKnownElement() {
			// Assert:
			AssertCopyInsertNewElement(123);
		}

		static void AssertBaseSetDeltaSupportsReinsertingExistingElement() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();

			// Act: find the matching element, update its dummy value, and reinsert it
			auto pOriginalElement = pDelta->find(TTraits::CreateKey("TestElement", 1));
			pOriginalElement->Dummy = 777;
			auto result = pDelta->insert(TTraits::ToSetElement(pOriginalElement));

			// - get the updated element
			auto pUpdatedElement = pDelta->find(TTraits::CreateKey("TestElement", 1));

			// Assert: it has been reinserted with the updated dummy value
			EXPECT_EQ(GetUpdatedResult(), result);

			EXPECT_EQ(777u, pUpdatedElement->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}

	private:
		static void AssertCopyInsertOriginalElement(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto pOriginalElement = pDelta->find(TTraits::CreateKey("TestElement", 2));
			pOriginalElement->Dummy = 123;
			auto initialResult = pDelta->insert(TTraits::ToSetElement(pOriginalElement));
			pDelta.commit(); // the 3 elements are now original elements

			// Sanity:
			EXPECT_EQ(GetUpdatedResult(), initialResult);

			// Arrange: find will insert a copy of the original element into the set of copies
			pDelta->find(TTraits::CreateKey("TestElement", 2));

			// - element is a clone of the above copied original element, but not the same object instance
			auto element = TTraits::CreateElement("TestElement", 2);

			// Act:
			std::vector<deltaset::InsertResult> results;
			for (auto i = 1u; i < 10; ++i) {
				TTraits::ToPointer(element)->Dummy = i;
				results.push_back(pDelta->insert(element));
			}

			auto pCurrent = pDelta->find(TTraits::CreateKey("TestElement", 2));

			// Assert:
			EXPECT_EQ(std::vector<deltaset::InsertResult>(results.size(), GetUpdatedResult()), results);

			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}

	public:
		static void AssertMutableBaseSetDeltaInsertUpdatesCopiedOriginalElement() {
			// Assert:
			AssertCopyInsertOriginalElement(9);
		}

		static void AssertImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalElement() {
			// Assert:
			AssertCopyInsertOriginalElement(123);
		}

	private:
		static void AssertCopyInsertOriginalElementAfterRemoval(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto pOriginalElement = pDelta->find(TTraits::CreateKey("TestElement", 2));
			pOriginalElement->Dummy = 123;
			pDelta->insert(TTraits::ToSetElement(pOriginalElement));
			pDelta.commit(); // the 3 elements are now original elements

			// - insert an (original) element
			auto element = TTraits::CreateElement("TestElement", 2);
			TTraits::ToPointer(element)->Dummy = 3;
			auto result1 = pDelta->insert(element);

			// - remove the copy
			pDelta->remove(TTraits::ToKey(element));

			// - reinsert the copy
			auto element2 = TTraits::CreateElement("TestElement", 2);
			TTraits::ToPointer(element2)->Dummy = 9;
			auto result2 = pDelta->insert(element2);

			// Act: get the current element
			auto pCurrent = pDelta->find(TTraits::CreateKey("TestElement", 2));

			// Assert:
			EXPECT_EQ(GetUpdatedResult(), result1);
			EXPECT_EQ(deltaset::InsertResult::Unremoved, result2);

			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}

	public:
		static void AssertMutableBaseSetDeltaInsertUpdatesCopiedOriginalElementAfterRemoval() {
			// Assert:
			AssertCopyInsertOriginalElementAfterRemoval(9);
		}

		static void AssertImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalElementAfterRemoval() {
			// Assert:
			AssertCopyInsertOriginalElementAfterRemoval(123);
		}

		static void AssertInsertOfSameAlreadyInsertedElementIsNullOperation() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto expectedElements = TTraits::CreateElements(3);

			// Act:
			std::vector<deltaset::InsertResult> results;
			for (auto i = 0u; i < 10; ++i)
				results.push_back(pDelta->emplace("TestElement", static_cast<unsigned int>(1)));

			// Assert:
			EXPECT_EQ(std::vector<deltaset::InsertResult>(results.size(), GetUpdatedResult()), results);

			EXPECT_EQ(3u, pDelta->size());
			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 0, 3, 0, 0);
		}

		static void AssertCanInsertRemovedElement() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			auto expectedElements = TTraits::CreateElements(3);
			auto removeResult = pDelta->remove(TTraits::CreateKey("TestElement", 1));

			// Sanity:
			EXPECT_EQ(deltaset::RemoveResult::Removed, removeResult);
			EXPECT_EQ(2u, pDelta->size());
			EXPECT_FALSE(!!pDelta->find(TTraits::CreateKey("TestElement", 1)));

			// Act:
			auto result = pDelta->emplace("TestElement", static_cast<unsigned int>(1));

			// Assert:
			EXPECT_EQ(deltaset::InsertResult::Unremoved, result);

			EXPECT_EQ(3u, pDelta->size());
			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}

		// endregion

		// region remove

		static void AssertCanRemoveExistingElement() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			auto element = TTraits::CreateElement("TestElement", 1);
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("TestElement", 0),
				TTraits::CreateElement("TestElement", 2)
			};

			// Act:
			auto result = pDelta->remove(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(deltaset::RemoveResult::Removed, result);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 3, 0, 1, 0);
		}

		static void AssertCanSubsequentlyRemoveExistingElements() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(5);
			pDelta.commit();
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("TestElement", 0),
				TTraits::CreateElement("TestElement", 2)
			};

			// Act:
			auto result1 = pDelta->remove(TTraits::CreateKey("TestElement", 1));
			auto result2 = pDelta->remove(TTraits::CreateKey("TestElement", 3));
			auto result3 = pDelta->remove(TTraits::CreateKey("TestElement", 4));

			// Assert:
			EXPECT_EQ(deltaset::RemoveResult::Removed, result1);
			EXPECT_EQ(deltaset::RemoveResult::Removed, result2);
			EXPECT_EQ(deltaset::RemoveResult::Removed, result3);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 5, 0, 3, 0);
		}

		static void AssertCanRemovePreviouslyInsertedElement() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			auto expectedElements = TTraits::CreateElements(3);
			auto insertResult = pDelta->insert(TTraits::CreateElement("MyTestElement", 234));

			// Sanity:
			EXPECT_EQ(insertResult, deltaset::InsertResult::Inserted);
			EXPECT_EQ(4u, pDelta->size());
			EXPECT_TRUE(!!pDelta->find(TTraits::CreateKey("MyTestElement", 234)));

			// Act:
			auto result = pDelta->remove(TTraits::CreateKey("MyTestElement", 234));

			// Assert:
			EXPECT_EQ(deltaset::RemoveResult::Uninserted, result);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 3, 0, 0, 0);
		}

		static void AssertCanRemoveMutatedElement() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			auto pElement = pDelta->find(TTraits::CreateKey("TestElement", 1));
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("TestElement", 0),
				TTraits::CreateElement("TestElement", 2)
			};

			// Act:
			pElement->Dummy = 456;
			auto result = pDelta->remove(TTraits::ToKey(TTraits::ToSetElement(pElement)));

			// Assert:
			using deltaset::RemoveResult;
			EXPECT_EQ(TTraits::IsElementMutable() ? RemoveResult::Unmodified_And_Removed : RemoveResult::Removed, result);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 3, 0, 1, 0);
		}

		static void AssertRemovingNonExistingElementDoesNotRemoveAnyElement() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			auto element = TTraits::CreateElement("TestElement", 4);
			auto expectedElements = TTraits::CreateElements(3);

			// Act:
			auto result = pDelta->remove(TTraits::ToKey(element));

			// Assert:
			EXPECT_EQ(deltaset::RemoveResult::None, result);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 3, 0, 0, 0);
		}

		static void AssertRemoveOfAlreadyRemovedElementIsNullOperation() {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			pDelta.commit();
			auto initialResult = pDelta->remove(TTraits::CreateKey("TestElement", 1));
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("TestElement", 0),
				TTraits::CreateElement("TestElement", 2)
			};

			// Sanity:
			EXPECT_EQ(deltaset::RemoveResult::Removed, initialResult);
			EXPECT_EQ(2u, pDelta->size());
			EXPECT_FALSE(!!pDelta->find(TTraits::CreateKey("TestElement", 1)));

			// Act:
			std::vector<deltaset::RemoveResult> results;
			for (auto i = 0u; i < 10; ++i)
				results.push_back(pDelta->remove(TTraits::CreateKey("TestElement", 1)));

			// Assert:
			EXPECT_EQ(std::vector<deltaset::RemoveResult>(results.size(), deltaset::RemoveResult::Redundant), results);

			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(pDelta, 3, 0, 1, 0);
		}

		// endregion

		// region reset

		static void AssertBaseSetDeltaResetClearsAllPendingChanges() {
			// Arrange:
			auto pDelta = CreateSetForBatchFindTests();

			// Sanity:
			AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 1 : 0);

			// Act:
			pDelta->reset();

			// Assert:
			AssertDeltaSizes(pDelta, 4, 0, 0, 0);
		}

		// endregion
	};

#define MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BaseSetDeltaTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_BASE_SET_DELTA_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_COMMON_BASE_SET_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_BASE_SET_DELTA_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, EmptyReturnsTrueIfAllOriginalElementsAreRemoved) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, EmptyReturnsFalseIfANonOriginalElementWasInserted) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, BaseSetDeltaFindConstReturnsOriginalElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, BaseSetDeltaFindConstReturnsConstCopy) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, BaseSetDeltaCanAccessAllElementsThroughFind) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, BaseSetDeltaCanAccessAllElementsThroughFindConst) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanInsertElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanInsertWithSuppliedParameters) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, BaseSetDeltaSupportsReinsertingExistingElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, InsertOfSameAlreadyInsertedElementIsNullOperation) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanInsertRemovedElement) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanRemoveExistingElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanSubsequentlyRemoveExistingElements) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanRemovePreviouslyInsertedElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, CanRemoveMutatedElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, RemovingNonExistingElementDoesNotRemoveAnyElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, RemoveOfAlreadyRemovedElementIsNullOperation) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, BaseSetDeltaResetClearsAllPendingChanges) \

#define DEFINE_MUTABLE_BASE_SET_DELTA_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_BASE_SET_DELTA_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, MutableBaseSetDeltaFindReturnsCopyForAnOriginalElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, MutableBaseSetDeltaFindReturnsNonConstCopy) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, MutableBaseSetDeltaInsertUpdatesKnownElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, MutableBaseSetDeltaInsertUpdatesCopiedOriginalElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, MutableBaseSetDeltaInsertUpdatesCopiedOriginalElementAfterRemoval) \

#define DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_BASE_SET_DELTA_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, ImmutableBaseSetDeltaFindReturnsOriginalElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, ImmutableBaseSetDeltaFindReturnsConstCopy) \
	\
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, ImmutableBaseSetDeltaInsertDoesNotUpdateKnownElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, ImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalElement) \
	MAKE_BASE_SET_DELTA_TEST(TEST_CLASS, TRAITS, ImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalElementAfterRemoval)

#define DEFINE_MUTABLE_BASE_SET_DELTA_TESTS_FOR(NAME) \
	DEFINE_MUTABLE_BASE_SET_DELTA_TESTS(Delta##NAME##Tests, test::DeltaTraits<deltaset::NAME##Traits>)

#define DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS_FOR(NAME) \
	DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS(Delta##NAME##Tests, test::DeltaTraits<deltaset::NAME##Traits>)
}}
