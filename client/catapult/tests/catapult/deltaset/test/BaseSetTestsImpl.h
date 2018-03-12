#pragma once
#include "BaseSetTestsInclude.h"
#include "catapult/exceptions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

#define DEFINE_BASE_SET_TESTS(TEST_NAME, TYPES) \
	template<typename TTraits> void TRAITS_TEST_NAME(BaseSetTests, TEST_NAME)(); \
	TYPES(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(BaseSetTests, TEST_NAME)()

#define MAKE_BASE_SET_TEST(TEST_NAME, TRAITS_TYPE, TRAITS_NAME) \
	TEST(BaseSetTests, TEST_NAME##_##TRAITS_NAME) { TRAITS_TEST_NAME(BaseSetTests, TEST_NAME)<TRAITS_TYPE>(); }

#define REGISTER_DELTA_TESTS(TEST_NAME) \
	REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME); \
	REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME); \

#define REGISTER_NON_DELTA_TESTS(TEST_NAME) \
	REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME); \
	REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME); \

#define REGISTER_ALL_TESTS(TEST_NAME) \
	REGISTER_DELTA_TESTS(TEST_NAME); \
	REGISTER_NON_DELTA_TESTS(TEST_NAME);

#define DEFINE_DELTA_MUTABLE_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_DELTA_MUTABLE_TYPES)
#define DEFINE_DELTA_IMMUTABLE_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_DELTA_IMMUTABLE_TYPES)
#define DEFINE_NON_DELTA_MUTABLE_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_NON_DELTA_MUTABLE_TYPES)

#define DEFINE_DELTA_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_DELTA_TESTS)
#define DEFINE_NON_DELTA_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_NON_DELTA_TESTS)
#define DEFINE_DELTA_AND_NON_DELTA_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_ALL_TESTS)

	// region multi-test helpers

	namespace {
		/// Dereferences pointer \a pElement.
		template<typename T>
		T Unwrap(T* pElement) {
			return *pElement;
		}

		template<typename T>
		T Unwrap(const std::shared_ptr<T>& pElement) {
			return *pElement;
		}

		/// Returns \c true if T is a shared_ptr, \c false otherwise.
		template<typename T>
		bool IsWrapped(T*) {
			return false;
		}

		template<typename T>
		bool IsWrapped(const std::shared_ptr<T>&) {
			return true;
		}

		/// Returns \c true if container is a map.
		template<typename T>
		bool IsMap(const T&) {
			return false;
		}

		template<typename TKey, typename TValue, typename THash>
		bool IsMap(const std::unordered_map<TKey, TValue, THash>&) {
			return true;
		}

		/// Returns \c true if container allows native value modification.
		template<typename T>
		bool AllowsNativeValueModification(const T&) {
			return IsMap(typename T::SetType());
		}

		/// Sets the dummy value of the element with \a value in \a set to \a dummy.
		template<typename TTraits>
		void SetDummyValue(const decltype(*TTraits::Create())& set, unsigned int value, size_t dummy) {
			// Act: find the matching element and update its dummy value
			auto pOriginalElement = set.find(TTraits::CreateKey("TestElement", value));
			pOriginalElement->Dummy = dummy;
		}

		/// Creates a set with all types of elements for batch find tests.
		template<typename TTraits>
		auto CreateSetForBatchFindTests() {
			auto pDelta = TTraits::CreateWithElements(4);
			pDelta.commit();
			pDelta->emplace("TestElement", 7u);
			pDelta->emplace("TestElement", 5u);
			pDelta->emplace("TestElement", 4u);
			pDelta->remove(TTraits::CreateKey("TestElement", 1u));
			pDelta->remove(TTraits::CreateKey("TestElement", 4u));
			SetDummyValue<TTraits>(*pDelta, 2, 42);
			SetDummyValue<TTraits>(*pDelta, 5, 42);
			return pDelta;
		}

		/// The expected elements in the set created by CreateSetForBatchFindTests.
		std::set<TestElement> CreateExpectedElementsForBatchFindTests() {
			// Assert:
			// + 0 -> original unmodified
			// - 1 -> original removed
			// + 2 -> original copied
			// + 3 -> original unmodified
			// - 4 -> inserted removed
			// + 5 -> inserted copied
			// + 7 -> inserted unmodified
			return {
				TestElement("TestElement", 0),
				TestElement("TestElement", 2),
				TestElement("TestElement", 3),
				TestElement("TestElement", 5),
				TestElement("TestElement", 7)
			};
		}

		/// Const qualifies \a pObject.
		template<typename T>
		std::shared_ptr<const T> MakeConst(const std::shared_ptr<T>& pObject) {
			return pObject;
		}

		/// Asserts that the delta sizes in \a deltaWrapper have the expected values
		/// (\a expectedOriginal, \a expectedAdded, \a expectedRemoved, \a expectedCopied).
		template<typename TDeltaWrapper>
		void AssertDeltaSizes(
				TDeltaWrapper& deltaWrapper,
				size_t expectedOriginal,
				size_t expectedAdded,
				size_t expectedRemoved,
				size_t expectedCopied) {
			// Act:
			auto deltas = deltaWrapper->deltas();

			// Assert:
			CATAPULT_LOG(debug) << "size: " << deltaWrapper.originalSize()
					<< " (O " << deltaWrapper.originalSize()
					<< ", A " << deltas.Added.size()
					<< ", R " << deltas.Removed.size()
					<< ", C " << deltas.Copied.size() << ")";
			EXPECT_EQ(expectedOriginal, deltaWrapper.originalSize());
			EXPECT_EQ(expectedAdded, deltas.Added.size());
			EXPECT_EQ(expectedRemoved, deltas.Removed.size());
			EXPECT_EQ(expectedCopied, deltas.Copied.size());
		}

		/// Asserts that the delta sizes in \a delta and the original size in \a set have the expected values
		/// (\a expectedOriginal, \a expectedAdded, \a expectedRemoved, \a expectedCopied).
		template<typename TSet, typename TDelta>
		void AssertDeltaSizes(
				const TSet& set,
				const TDelta& delta,
				size_t expectedOriginal,
				size_t expectedAdded,
				size_t expectedRemoved,
				size_t expectedCopied) {
			// Act:
			auto deltas = delta.deltas();

			// Assert:
			CATAPULT_LOG(debug) << "size: " << set.size()
					<< " (O " << set.size()
					<< ", A " << deltas.Added.size()
					<< ", R " << deltas.Removed.size()
					<< ", C " << deltas.Copied.size() << ")";
			EXPECT_EQ(expectedOriginal, set.size());
			EXPECT_EQ(expectedAdded, deltas.Added.size());
			EXPECT_EQ(expectedRemoved, deltas.Removed.size());
			EXPECT_EQ(expectedCopied, deltas.Copied.size());
		}
	}

	// endregion

	// region ctor

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanCreateBaseSet) {
		// Act:
		auto pSet = TTraits::Create();

		// Assert:
		TTraits::AssertContents(*pSet, typename TTraits::ElementVector());
	}

	// endregion

	// region empty

	DEFINE_DELTA_AND_NON_DELTA_TESTS(EmptyReturnsTrueForEmptySet) {
		// Act:
		auto pSet = TTraits::Create();

		// Assert:
		EXPECT_TRUE(pSet->empty());
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(EmptyReturnsFalseForNonEmptySet) {
		// Act:
		auto pSet = TTraits::CreateWithElements(3);

		// Assert:
		EXPECT_FALSE(pSet->empty());
	}

	DEFINE_DELTA_IMMUTABLE_TESTS(EmptyReturnsTrueIfAllOriginalElementsAreRemoved) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		pDelta.commit();
		for (auto i = 0u; i < 3; i++)
			pDelta->remove(TTraits::CreateKey("TestElement", i));

		// Assert:
		EXPECT_TRUE(pDelta->empty());
		AssertDeltaSizes(pDelta, 3, 0, 3, 0);
	}

	DEFINE_DELTA_IMMUTABLE_TESTS(EmptyReturnsFalseIfANonOriginalElementWasInserted) {
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

	// region find / findIterator

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanFindExistingElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element = TTraits::CreateElement("TestElement", 2);

		// Act:
		auto pFoundElement = pSet->find(TTraits::ToKey(element));

		// Assert:
		EXPECT_EQ(3u, pSet->size());
		ASSERT_TRUE(!!pFoundElement);
		EXPECT_EQ(*TTraits::ToPointer(element), *pFoundElement);
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanFindExistingElementMultipleTimes) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element = TTraits::CreateElement("TestElement", 2);

		// Act:
		auto pFoundElement1 = pSet->find(TTraits::ToKey(element));
		auto pFoundElement2 = pSet->find(TTraits::ToKey(element));

		// Assert:
		EXPECT_EQ(3u, pSet->size());
		ASSERT_TRUE(!!pFoundElement1);
		ASSERT_TRUE(!!pFoundElement2);
		EXPECT_EQ(pFoundElement1, pFoundElement2);
		EXPECT_EQ(*TTraits::ToPointer(element), *pFoundElement1);
		EXPECT_EQ(*TTraits::ToPointer(element), *pFoundElement2);
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CannotFindNonExistingElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element1 = TTraits::CreateElement("TestElement", 123);
		auto element2 = TTraits::CreateElement("BadElement", 2);

		// Act:
		auto pFoundElement1 = pSet->find(TTraits::ToKey(element1));
		auto pFoundElement2 = pSet->find(TTraits::ToKey(element2));

		// Assert:
		EXPECT_EQ(3u, pSet->size());
		EXPECT_FALSE(!!pFoundElement1);
		EXPECT_FALSE(!!pFoundElement2);
	}

	DEFINE_DELTA_MUTABLE_TESTS(MutableBaseSetDeltaFindReturnsCopyForAnOriginalElement) {
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

	DEFINE_DELTA_IMMUTABLE_TESTS(ImmutableBaseSetDeltaFindReturnsOriginalElement) {
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

	DEFINE_DELTA_MUTABLE_TESTS(MutableBaseSetDeltaFindReturnsNonConstCopy) {
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

	DEFINE_DELTA_IMMUTABLE_TESTS(ImmutableBaseSetDeltaFindReturnsConstCopy) {
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

	DEFINE_DELTA_TESTS(BaseSetDeltaFindConstReturnsOriginalElement) {
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

	DEFINE_DELTA_TESTS(BaseSetDeltaFindConstReturnsConstCopy) {
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

	namespace {
		template<typename TTraits, typename TSet>
		void AssertBatchFind(TSet& set) {
			// Assert:
			auto expectedElements = CreateExpectedElementsForBatchFindTests();

			for (const auto& element : expectedElements)
				EXPECT_TRUE(!!set.find(TTraits::CreateKey(element.Name, element.Value))) << element;

			for (auto value : { 1u, 4u, 6u, 8u, 9u })
				EXPECT_FALSE(!!set.find(TTraits::CreateKey("TestElement", value))) << value;

			// Sanity:
			EXPECT_EQ(expectedElements.size(), set.size());
		}
	}

	DEFINE_DELTA_TESTS(BaseSetDeltaCanAccessAllElementsThroughFind) {
		// Arrange:
		auto pDelta = CreateSetForBatchFindTests<TTraits>();

		// Assert: note that (non-const) find will make copies of all elements
		AssertBatchFind<TTraits, typename decltype(pDelta)::DeltaType>(*pDelta);
		AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 3 : 0);
	}

	DEFINE_DELTA_TESTS(BaseSetDeltaCanAccessAllElementsThroughFindConst) {
		// Arrange:
		auto pDelta = CreateSetForBatchFindTests<TTraits>();

		// Assert:
		AssertBatchFind<TTraits, const typename decltype(pDelta)::DeltaType>(*pDelta);
		AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 1 : 0);
	}

	DEFINE_NON_DELTA_TESTS(FindIteratorReturnsIteratorToElementIfElementExists) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element = TTraits::CreateElement("TestElement", 1);

		// Act:
		auto iter = pSet->findIterator(TTraits::ToKey(element));

		// Assert:
		EXPECT_EQ(*TTraits::ToPointer(element), *TTraits::ToPointerFromStorage(*iter));
	}

	DEFINE_NON_DELTA_TESTS(FindIteratorReturnsCendIfElementDoesNotExist) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element = TTraits::CreateElement("TestElement", 4);

		// Act:
		auto iter = pSet->findIterator(TTraits::ToKey(element));

		// Assert:
		EXPECT_EQ(pSet->end(), iter);
	}

	// endregion

	// region contains

	DEFINE_DELTA_AND_NON_DELTA_TESTS(ContainsReturnsTrueForExistingElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element = TTraits::CreateElement("TestElement", 2);

		// Act:
		auto found1 = pSet->contains(TTraits::ToKey(element));

		// Assert:
		EXPECT_TRUE(found1);
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(ContainsReturnsFalseForNonExistingElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto element1 = TTraits::CreateElement("TestElement", 123);
		auto element2 = TTraits::CreateElement("BadElement", 2);

		// Act:
		auto found1 = pSet->contains(TTraits::ToKey(element1));
		auto found2 = pSet->contains(TTraits::ToKey(element2));

		// Assert:
		EXPECT_FALSE(found1);
		EXPECT_FALSE(found2);
	}

	// endregion

	// region BaseSet iterators and set order

	namespace {
		template<typename TBaseSet>
		void AssertConstIterator(const TBaseSet& set) {
			// Assert: the unwrapped set iterator element pointer points to a const element
			auto iter = set.begin();
			EXPECT_TRUE(std::is_const<decltype(Unwrap(iter.operator->()))>());
		}
	}

	namespace {
		template<typename TTraits>
		void AssertBeginEndIteratorConsistency(const decltype(*TTraits::Create())& set) {
			// Sanity: ensure cbegin and cend iterators are only equal when the set is empty
			if (set.empty())
				EXPECT_EQ(set.begin(), set.end());
			else
				EXPECT_NE(set.begin(), set.end());
		}

		template<typename TTraits>
		std::set<TestElement> ExtractElements(const decltype(*TTraits::Create())& set) {
			// Act: iterate over the set and extract all elements
			std::set<TestElement> elements;
			size_t numIteratedElements = 0;
			for (const auto& element : set) {
				elements.insert(*TTraits::ToPointerFromStorage(element));
				++numIteratedElements;
			}

			// Sanity: iterators are consistent with size and there are no duplicate elements in the set
			AssertBeginEndIteratorConsistency<TTraits>(set);
			EXPECT_EQ(set.size(), numIteratedElements);
			return elements;
		}

		template<typename TTraits>
		void AssertIteration(const decltype(*TTraits::Create())& set, const std::set<TestElement>& expectedElements) {
			// Act:
			auto actualElements = ExtractElements<TTraits>(set);

			// Assert:
			EXPECT_EQ(expectedElements, actualElements);

			// Sanity: the iterator elements are const
			AssertConstIterator(set);
		}
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanIterateThroughEmptySetWithConstIterator) {
		// Assert:
		AssertIteration<TTraits>(*TTraits::Create(), {});
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanIterateThroughSingleValueSetWithConstIterator) {
		// Assert:
		AssertIteration<TTraits>(*TTraits::CreateWithElements(1), { TestElement("TestElement", 0) });
	}

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanIterateThroughMultiValueSetWithConstIterator) {
		// Assert:
		std::set<TestElement> expectedElements{
			TestElement("TestElement", 0),
			TestElement("TestElement", 1),
			TestElement("TestElement", 2)
		};
		AssertIteration<TTraits>(*TTraits::CreateWithElements(3), expectedElements);
	}

	DEFINE_DELTA_TESTS(DeltaIterationIncludesOriginalElements) {
		// Arrange: commit so that all elements become original elements
		auto pDelta = TTraits::CreateWithElements(3);
		pDelta.commit();

		// Act:
		auto actualElements = ExtractElements<TTraits>(*pDelta);

		// Assert:
		std::set<TestElement> expectedElements{
			TestElement("TestElement", 0),
			TestElement("TestElement", 1),
			TestElement("TestElement", 2)
		};
		EXPECT_EQ(expectedElements, actualElements);
		AssertDeltaSizes(pDelta, 3, 0, 0, 0);
	}

	DEFINE_DELTA_TESTS(DeltaIterationCanIterateOnlyInsertedElements) {
		// Arrange:
		auto pDelta = TTraits::Create();
		pDelta->emplace("TestElement", 7u);
		pDelta->emplace("TestElement", 4u);

		// Act:
		auto actualElements = ExtractElements<TTraits>(*pDelta);

		// Assert:
		std::set<TestElement> expectedElements{
			TestElement("TestElement", 4),
			TestElement("TestElement", 7)
		};
		EXPECT_EQ(expectedElements, actualElements);
		AssertDeltaSizes(pDelta, 0, 2, 0, 0);
	}

	DEFINE_DELTA_TESTS(DeltaIterationIncludesInsertedElements) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		pDelta.commit();
		pDelta->emplace("TestElement", 7u);
		pDelta->emplace("TestElement", 4u);

		// Act:
		auto actualElements = ExtractElements<TTraits>(*pDelta);

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

	DEFINE_DELTA_TESTS(DeltaIterationExcludesRemovedElements) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		pDelta.commit();
		pDelta->emplace("TestElement", 7u);
		pDelta->emplace("TestElement", 4u);
		pDelta->remove(TTraits::CreateKey("TestElement", 1u));
		pDelta->remove(TTraits::CreateKey("TestElement", 4u));

		// Act:
		auto actualElements = ExtractElements<TTraits>(*pDelta);

		// Assert:
		std::set<TestElement> expectedElements{
			TestElement("TestElement", 0),
			TestElement("TestElement", 2),
			TestElement("TestElement", 7)
		};
		EXPECT_EQ(expectedElements, actualElements);
		AssertDeltaSizes(pDelta, 3, 1, 1, 0);
	}

	namespace {
		template<typename TTraits>
		std::set<size_t> ExtractDummyValues(const decltype(*TTraits::Create())& set) {
			std::set<size_t> dummyValues;
			size_t numIteratedElements = 0;
			for (const auto& element : set) {
				dummyValues.insert(TTraits::ToPointerFromStorage(element)->Dummy);
				++numIteratedElements;
			}

			// Sanity: iterators are consistent with size and there are no duplicate elements in the set
			AssertBeginEndIteratorConsistency<TTraits>(set);
			EXPECT_EQ(set.size(), numIteratedElements);
			return dummyValues;
		}
	}

	DEFINE_DELTA_TESTS(DeltaIterationIncludesCopiedElements) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		SetDummyValue<TTraits>(*pDelta, 0, 0);
		SetDummyValue<TTraits>(*pDelta, 1, 1);
		SetDummyValue<TTraits>(*pDelta, 2, 2);
		pDelta.commit(); // the 3 elements are now original elements with dummy values set

		// - change the dummy value for one element
		SetDummyValue<TTraits>(*pDelta, 1, 42);

		// Act:
		auto actualDummyValues = ExtractDummyValues<TTraits>(*pDelta);

		// Assert: iterating should pick up the new dummy value
		//         (in the case of mutable, the copied element will be returned in place of the original element)
		//         (in the case of immutable, the original element with the changed value will be returned)
		std::set<size_t> expectedDummyValues{ 0, 42, 2 };
		EXPECT_EQ(expectedDummyValues, actualDummyValues);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
	}

	DEFINE_DELTA_TESTS(DeltaIterationCanIterateOnlyCopiedElements) {
		// Arrange:
		constexpr auto Num_Elements = 3u;
		auto pDelta = TTraits::CreateWithElements(Num_Elements);
		for (auto i = 0u; i < Num_Elements; ++i)
			SetDummyValue<TTraits>(*pDelta, i, i);
		pDelta.commit(); // the 3 elements are now original elements with dummy values set

		// - make copies of all the elements
		for (auto i = 0u; i < Num_Elements; ++i)
			SetDummyValue<TTraits>(*pDelta, i, i + 7);

		// Act:
		auto actualDummyValues = ExtractDummyValues<TTraits>(*pDelta);

		// Assert: iterating should pick up the new dummy values
		std::set<size_t> expectedDummyValues{ 7, 8, 9 };
		EXPECT_EQ(expectedDummyValues, actualDummyValues);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 3 : 0);
	}

	DEFINE_DELTA_TESTS(DeltaIterationDoesNotReiterateNewlyCopiedElements) {
		// Arrange: create a set with three original elements with dummy values
		auto pDelta = TTraits::CreateWithElements(3);
		SetDummyValue<TTraits>(*pDelta, 0, 0);
		SetDummyValue<TTraits>(*pDelta, 1, 1);
		SetDummyValue<TTraits>(*pDelta, 2, 2);
		pDelta.commit(); // the 3 elements are now original elements with dummy values set

		// - add a fourth (uncommitted) element with a dummy value
		pDelta->emplace("TestElement", 3u);
		SetDummyValue<TTraits>(*pDelta, 3, 3);

		// Act: iterate
		std::set<size_t> dummyValues;
		size_t numIteratedElements = 0;
		for (const auto& element : *pDelta) {
			auto pCurrentElement = TTraits::ToPointerFromStorage(element);
			auto dummyValue = pCurrentElement->Dummy;
			dummyValues.insert(dummyValue);
			++numIteratedElements;

			// - change the dummy value of the middle elements
			if (1 > dummyValue || dummyValue > 2) continue;
			auto pElementCopy = pDelta->find(TTraits::CreateKey(pCurrentElement->Name, pCurrentElement->Value));
			pElementCopy->Dummy = dummyValue + 40;
		}

		// Assert: initial iteration should pick up the old dummy values
		std::set<size_t> expectedDummyValues{ 0, 1, 2, 3 };
		EXPECT_EQ(expectedDummyValues.size(), numIteratedElements);
		EXPECT_EQ(expectedDummyValues, dummyValues);

		// Sanity: reiterating should pick up the new dummy values
		expectedDummyValues = { 0, 41, 42, 3 };
		EXPECT_EQ(expectedDummyValues, ExtractDummyValues<TTraits>(*pDelta));
		AssertDeltaSizes(pDelta, 3, 1, 0, TTraits::IsElementMutable() ? 2 : 0);
	}

	DEFINE_DELTA_TESTS(DeltaCannotDereferenceAtEnd) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		auto iter = pDelta->end();

		// Act + Assert:
		EXPECT_THROW(*iter, catapult_out_of_range);
		EXPECT_THROW(iter.operator->(), catapult_out_of_range);
	}

	DEFINE_DELTA_TESTS(DeltaCannotAdvancePrefixIteratorBeyondEnd) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		auto iter = pDelta->end();

		// Act + Assert:
		EXPECT_THROW(++iter, catapult_out_of_range);
		EXPECT_THROW(++iter, catapult_out_of_range);
	}

	DEFINE_DELTA_TESTS(DeltaCannotAdvancePostfixIteratorBeyondEnd) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		auto iter = pDelta->end();

		// Act + Assert:
		EXPECT_THROW(iter++, catapult_out_of_range);
		EXPECT_THROW(iter++, catapult_out_of_range);
	}

	DEFINE_DELTA_TESTS(DeltaBeginEndIteratorsBasedOnSameContainerAreEqual) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);

		// Act + Assert:
		EXPECT_EQ(pDelta->begin(), pDelta->begin());
		EXPECT_EQ(pDelta->end(), pDelta->end());
		EXPECT_NE(pDelta->begin(), pDelta->end());
	}

	DEFINE_DELTA_TESTS(DeltaBeginEndIteratorsBasedOnDifferentContainerAreNotEqual) {
		// Arrange:
		auto pDelta1 = TTraits::CreateWithElements(3);
		auto pDelta2 = TTraits::CreateWithElements(3);

		// Act + Assert:
		EXPECT_NE(pDelta1->begin(), pDelta2->begin());
		EXPECT_NE(pDelta1->end(), pDelta2->end());
	}

	namespace {
		template<typename TTraits>
		void AssertDeltaIteration(const consumer<decltype(TTraits::Create()->begin())&>& increment) {
			// Arrange:
			auto pDelta = CreateSetForBatchFindTests<TTraits>();

			// Act:
			std::set<TestElement> actualElements;
			size_t numIteratedElements = 0;
			for (auto iter = pDelta->begin(); pDelta->end() != iter; increment(iter)) {
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
			EXPECT_TRUE(std::is_const<typename decltype(pDelta->begin())::value_type>());
			AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 1 : 0);
		}
	}

	DEFINE_DELTA_TESTS(DeltaIterationSupportsPostfixIterator) {
		AssertDeltaIteration<TTraits>([](auto& iter) { iter++; });
	}

	DEFINE_DELTA_TESTS(DeltaIterationSupportsPrefixIterator) {
		// Assert:
		AssertDeltaIteration<TTraits>([](auto& iter) { ++iter; });
	}

	// endregion

	// region insert

	template<typename TTraits>
	deltaset::InsertResult GetUpdatedResult() {
		// mutable elements can be updated, immutable ones cannot by definition
		return TTraits::IsElementMutable() ? deltaset::InsertResult::Updated : deltaset::InsertResult::Redundant;
	}

	DEFINE_DELTA_TESTS(CanInsertElement) {
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

	DEFINE_DELTA_TESTS(CanInsertWithSuppliedParameters) {
		// Arrange:
		auto pDelta = TTraits::Create();
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("MyTestElement", 0),
			TTraits::CreateElement("AnotherTestElement", 123)
		};

		// Act:
		auto result1 = pDelta->emplace("MyTestElement");
		auto result2 = pDelta->emplace("AnotherTestElement", 123u);

		// Assert:
		EXPECT_EQ(deltaset::InsertResult::Inserted, result1);
		EXPECT_EQ(deltaset::InsertResult::Inserted, result2);

		TTraits::AssertContents(*pDelta, expectedElements);
		AssertDeltaSizes(pDelta, 0, 2, 0, 0);
	}

	namespace {
		template<typename TTraits>
		void AssertCopyInsertNewElement(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::Create();
			auto element = TTraits::CreateElement("TestElement", 4);
			TTraits::ToPointer(element)->Dummy = 123;
			auto initialResult = pDelta->insert(element); // pElement is not an original element

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
			EXPECT_EQ(std::vector<deltaset::InsertResult>(results.size(), GetUpdatedResult<TTraits>()), results);

			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 0, 1, 0, 0);
		}
	}

	DEFINE_DELTA_MUTABLE_TESTS(MutableBaseSetDeltaInsertUpdatesKnownElement) {
		// Assert:
		AssertCopyInsertNewElement<TTraits>(9u);
	}

	DEFINE_DELTA_IMMUTABLE_TESTS(ImmutableBaseSetDeltaInsertDoesNotUpdateKnownElement) {
		// Assert:
		AssertCopyInsertNewElement<TTraits>(123u);
	}

	DEFINE_DELTA_TESTS(BaseSetDeltaSupportsReinsertingExistingElement) {
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
		EXPECT_EQ(GetUpdatedResult<TTraits>(), result);

		EXPECT_EQ(777u, pUpdatedElement->Dummy);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
	}

	namespace {
		template<typename TTraits>
		void AssertCopyInsertOriginalElement(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::CreateWithElements(3);
			auto pOriginalElement = pDelta->find(TTraits::CreateKey("TestElement", 2));
			pOriginalElement->Dummy = 123;
			auto initialResult = pDelta->insert(TTraits::ToSetElement(pOriginalElement));
			pDelta.commit(); // the 3 elements are now original elements

			// Sanity:
			EXPECT_EQ(GetUpdatedResult<TTraits>(), initialResult);

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
			EXPECT_EQ(std::vector<deltaset::InsertResult>(results.size(), GetUpdatedResult<TTraits>()), results);

			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}
	}

	DEFINE_DELTA_MUTABLE_TESTS(MutableBaseSetDeltaInsertUpdatesCopiedOriginalElement) {
		// Assert:
		AssertCopyInsertOriginalElement<TTraits>(9u);
	}

	DEFINE_DELTA_IMMUTABLE_TESTS(ImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalElement) {
		// Assert:
		AssertCopyInsertOriginalElement<TTraits>(123u);
	}

	namespace {
		template<typename TTraits>
		void AssertCopyInsertOriginalElementAfterRemoval(uint32_t expectedValue) {
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
			EXPECT_EQ(GetUpdatedResult<TTraits>(), result1);
			EXPECT_EQ(deltaset::InsertResult::Unremoved, result2);

			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
		}
	}

	DEFINE_DELTA_MUTABLE_TESTS(MutableBaseSetDeltaInsertUpdatesCopiedOriginalElementAfterRemoval) {
		// Assert:
		AssertCopyInsertOriginalElementAfterRemoval<TTraits>(9u);
	}

	DEFINE_DELTA_IMMUTABLE_TESTS(ImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalElementAfterRemoval) {
		// Assert:
		AssertCopyInsertOriginalElementAfterRemoval<TTraits>(123u);
	}

	DEFINE_NON_DELTA_TESTS(InsertDoesNotChangeOriginalBaseSet) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		auto result = pDelta->emplace("MyElement", 123u);

		// Assert:
		EXPECT_EQ(deltaset::InsertResult::Inserted, result);

		EXPECT_EQ(4u, pDelta->size());
		TTraits::AssertContents(*pBaseSet, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 1, 0, 0);
	}

	DEFINE_DELTA_TESTS(InsertOfSameAlreadyInsertedElementIsNullOperation) {
		// Arrange:
		auto pDelta = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		std::vector<deltaset::InsertResult> results;
		for (auto i = 0u; i < 10; ++i)
			results.push_back(pDelta->emplace("TestElement", 1u));

		// Assert:
		EXPECT_EQ(std::vector<deltaset::InsertResult>(results.size(), GetUpdatedResult<TTraits>()), results);

		EXPECT_EQ(3u, pDelta->size());
		TTraits::AssertContents(*pDelta, expectedElements);
		AssertDeltaSizes(pDelta, 0, 3, 0, 0);
	}

	DEFINE_DELTA_TESTS(CanInsertRemovedElement) {
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
		auto result = pDelta->emplace("TestElement", 1u);

		// Assert:
		EXPECT_EQ(deltaset::InsertResult::Unremoved, result);

		EXPECT_EQ(3u, pDelta->size());
		TTraits::AssertContents(*pDelta, expectedElements);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsElementMutable() ? 1 : 0);
	}

	// endregion

	// region remove

	DEFINE_DELTA_TESTS(CanRemoveExistingElement) {
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

	DEFINE_DELTA_TESTS(CanSubsequentlyRemoveExistingElements) {
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

	DEFINE_DELTA_TESTS(CanRemovePreviouslyInsertedElement) {
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

	DEFINE_DELTA_TESTS(CanRemoveMutatedElement) {
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
		EXPECT_EQ(TTraits::IsElementMutable() ? deltaset::RemoveResult::Unmodified_And_Removed : deltaset::RemoveResult::Removed, result);

		TTraits::AssertContents(*pDelta, expectedElements);
		AssertDeltaSizes(pDelta, 3, 0, 1, 0);
	}

	DEFINE_DELTA_TESTS(RemovingNonExistingElementDoesNotRemoveAnyElement) {
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

	DEFINE_DELTA_TESTS(RemoveOfAlreadyRemovedElementIsNullOperation) {
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

	DEFINE_NON_DELTA_TESTS(RemoveDoesNotChangeOriginalBaseSet) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		auto result = pDelta->remove(TTraits::CreateKey("TestElement", 2));

		// Assert:
		EXPECT_EQ(deltaset::RemoveResult::Removed, result);

		EXPECT_EQ(2u, pDelta->size());
		TTraits::AssertContents(*pBaseSet, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 1, 0);
	}

	// endregion

	// region reset

	DEFINE_DELTA_TESTS(BaseSetDeltaResetClearsAllPendingChanges) {
		// Arrange:
		auto pDelta = CreateSetForBatchFindTests<TTraits>();

		// Sanity:
		AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsElementMutable() ? 1 : 0);

		// Act:
		pDelta->reset();

		// Assert:
		AssertDeltaSizes(pDelta, 4, 0, 0, 0);
	}

	// endregion

	// region rebase / rebaseDetached

	DEFINE_NON_DELTA_TESTS(RebaseCreatesDeltaAroundSuppliedElements) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		auto pDelta = pBaseSet->rebase();

		// Assert:
		TTraits::AssertContents(*pDelta, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
	}

	DEFINE_NON_DELTA_TESTS(RebaseAllowsOnlyOneAttachedDeltaAtATime) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);
		{
			auto pDelta = pBaseSet->rebase();

			// Act + Assert:
			EXPECT_THROW(pBaseSet->rebase(), catapult_runtime_error);
		}

		// Act: delta went out of scope, another delta is allowed
		auto pDelta = pBaseSet->rebase();
		TTraits::AssertContents(*pDelta, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
	}

	DEFINE_NON_DELTA_TESTS(RebaseDetachedCreatesDeltaAroundSuppliedElements) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		auto pDetachedDelta = pBaseSet->rebaseDetached();

		// Assert:
		TTraits::AssertContents(*pDetachedDelta, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDetachedDelta, 3, 0, 0, 0);
	}

	DEFINE_NON_DELTA_TESTS(RebaseDetachedAllowsManyDeltas) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);
		std::vector<decltype(pBaseSet->rebaseDetached())> deltas;

		// Act:
		for (auto i = 0u; i < 10; ++i)
			deltas.push_back(pBaseSet->rebaseDetached());

		// Assert:
		for (const auto& pDetachedDelta : deltas) {
			TTraits::AssertContents(*pDetachedDelta, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDetachedDelta, 3, 0, 0, 0);
		}
	}

	// endregion

	// region commit

	DEFINE_NON_DELTA_TESTS(CannotCommitWhenThereAreNoPendingAttachedDeltas) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);

		// Act + Assert:
		EXPECT_THROW(pBaseSet->commit(), catapult_runtime_error);
	}

	DEFINE_NON_DELTA_TESTS(CommitThrowsIfOnlyDetachedDeltasAreOutstanding) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDetachedDelta = pSet->rebaseDetached();

		// Act + Assert:
		EXPECT_THROW(pSet->commit(), catapult_runtime_error);
	}

	DEFINE_NON_DELTA_TESTS(CommitCommitsToOriginalElements) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("MyTestElement", 123u);
		pDelta->remove(TTraits::CreateKey("TestElement", 0));
		pDelta->remove(TTraits::CreateKey("TestElement", 2));
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 1),
			TTraits::CreateElement("MyTestElement", 123),
		};

		// Act:
		pBaseSet->commit();

		// Assert:
		TTraits::AssertContents(*pBaseSet, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDelta, 2, 0, 0, 0);
	}

	DEFINE_NON_DELTA_MUTABLE_TESTS(CommitReflectsChangesOnOriginalElements) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		auto pElement = pDelta->find(TTraits::CreateKey("TestElement", 1));

		// Act:
		pElement->Dummy = 123;
		pBaseSet->commit();
		auto pElementAfterCommit = pBaseSet->find(TTraits::CreateKey("TestElement", 1));

			// Assert:
		EXPECT_EQ(123u, pElementAfterCommit->Dummy);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
	}

	DEFINE_NON_DELTA_TESTS(CommitIsIdempotent) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("MyTestElement", 123u);
		pDelta->remove(TTraits::CreateKey("TestElement", 0));
		pDelta->remove(TTraits::CreateKey("TestElement", 2));
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 1),
			TTraits::CreateElement("MyTestElement", 123),
		};

		// Act:
		for (auto i = 0u; i < 5u; ++i)
			pBaseSet->commit();

		// Assert:
		TTraits::AssertContents(*pBaseSet, expectedElements);
		AssertDeltaSizes(*pBaseSet, *pDelta, 2, 0, 0, 0);
	}

	// endregion
}}
