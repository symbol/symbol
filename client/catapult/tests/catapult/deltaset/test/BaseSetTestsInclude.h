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
#include "ContainerDeltaPair.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/BaseSetDefaultTraits.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/deltaset/OrderedSet.h"
#include "catapult/utils/traits/StlTraits.h"
#include "tests/TestHarness.h"
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace test {

	// region TestElement

	/// Test element used as the base of both immutable and mutable test elements.
	struct TestElement {
	public:
		TestElement() : TestElement("TestElement", 0)
		{}

		explicit TestElement(const std::string& name) : TestElement(name, 0)
		{}

		explicit TestElement(unsigned int value) : TestElement("TestElement", value)
		{}

		explicit TestElement(const std::string& name, unsigned int value)
				: Name(name)
				, Value(value)
				, Hash(calculateHash())
				, Dummy(0)
				, HasherCallCount(0)
		{}

	public:
		bool operator<(const TestElement& rhs) const {
			return Name < rhs.Name || (Name == rhs.Name && Value < rhs.Value);
		}

		bool operator==(const TestElement& rhs) const {
			return Name == rhs.Name && Value == rhs.Value;
		}

	public:
		friend std::ostream& operator<<(std::ostream& out, const TestElement& element) {
			out << "TestElement(" << element.Name << ", " << element.Value << ")" << std::endl;
			return out;
		}

	public:
		size_t calculateHash() {
			auto raw = Name + std::to_string(Value);
			auto checksum = 0u;
			for (auto ch : raw)
				checksum += static_cast<size_t>(ch);

			return checksum;
		}

	public:
		std::string Name;
		unsigned int Value;
		size_t Hash;
		mutable size_t Dummy;
		mutable size_t HasherCallCount;
	};

	/// Immutable test element (deltas should not copy on write).
	struct ImmutableTestElement : public TestElement {
		using TestElement::TestElement;
	};

	/// Mutable test element (deltas should copy on write).
	struct MutableTestElement : public TestElement {
		using TestElement::TestElement;

		void mutate()
		{}
	};

	// endregion

	// region composite classes used in BaseSet(Delta) definitions

	/// Hashing function for TestElement.
	template<typename T>
	struct Hasher {
		size_t operator()(const T& element) const {
			++element.HasherCallCount;
			return element.Hash;
		}
	};

	template<typename T>
	struct Hasher<std::shared_ptr<T>> {
		size_t operator()(const std::shared_ptr<T>& pElement) const {
			Hasher<T> hasher;
			return hasher(*pElement);
		}
	};

	/// Checks two objects of the same type for equality.
	template<typename T>
	struct EqualityChecker {
		bool operator()(const T& lhs, const T& rhs) const {
			return lhs == rhs;
		}
	};

	template<typename T>
	struct EqualityChecker<std::shared_ptr<T>> {
		bool operator()(const std::shared_ptr<T>& pLhs, const std::shared_ptr<T>& pRhs) const {
			EqualityChecker<T> equalityChecker;
			return equalityChecker(*pLhs, *pRhs);
		}
	};

	/// Reverse comparator that reverses the natural sorting order of elements.
	template<typename T>
	struct ReverseComparator {
		bool operator()(const T& lhs, const T& rhs) const {
			return rhs < lhs;
		}
	};

	template<typename T>
	struct ReverseComparator<std::shared_ptr<T>> {
		bool operator()(const std::shared_ptr<T>& pLhs, const std::shared_ptr<T>& pRhs) const {
			ReverseComparator<T> comparator;
			return comparator(*pLhs, *pRhs);
		}
	};

	// endregion

	// region IsMutable / IsMap

	/// Returns \c true if \a TElement is mutable.
	template<typename TElement>
	constexpr bool IsMutable() {
		return false;
	}

	template<>
	constexpr bool IsMutable<MutableTestElement>() {
		return true;
	}

	template<>
	constexpr bool IsMutable<std::shared_ptr<MutableTestElement>>() {
		return true;
	}

	/// Returns \c true if container is a map.
	template<typename T>
	bool IsMap(const T&) {
		return utils::traits::is_map<T>::value;
	}

	// endregion

	// region ElementFactory

	/// Factory for creating and converting elements.
	template<typename T>
	struct ElementFactory {
		using ElementType = T;

		static T CreateElement(const std::string& name, unsigned int value) {
			return T(name, value);
		}

		template<typename TElement>
		static TElement* ToPointer(TElement& element) {
			return &element;
		}

		template<typename TElement>
		static TElement& ToSetElement(TElement* pElement) {
			return *pElement;
		}
	};

	template<typename T>
	struct ElementFactory<std::shared_ptr<T>> {
		using ElementType = std::shared_ptr<T>;

		static std::shared_ptr<T> CreateElement(const std::string& name, unsigned int value) {
			return std::make_shared<T>(name, value);
		}

		static std::shared_ptr<T> ToPointer(const std::shared_ptr<T>& pElement) {
			return pElement;
		}

		static std::shared_ptr<T> ToSetElement(const std::shared_ptr<T>& pElement) {
			return pElement;
		}
	};

	/// Extends ElementFactory by adding batch functions.
	template<typename TBaseSetTraits>
	struct BatchElementFactory : public ElementFactory<typename TBaseSetTraits::ElementType> {
		using BaseType = ElementFactory<typename TBaseSetTraits::ElementType>;
		using ElementType = typename BaseType::ElementType;
		using ElementVector = std::vector<typename std::remove_const<ElementType>::type>;

		static ElementVector CreateElements(size_t count) {
			ElementVector elements;
			for (auto i = 0u; i < count; ++i)
				elements.push_back(BaseType::CreateElement("TestElement", i));

			return elements;
		}

		template<typename TBaseSetDelta>
		static void InsertElements(TBaseSetDelta& delta, size_t count) {
			for (auto i = 0u; i < count; ++i)
				delta.insert(BaseType::CreateElement("TestElement", i));
		}

		template<typename TSet>
		static void AssertContents(const TSet& set, const ElementVector& expectedElements) {
			ASSERT_EQ(expectedElements.size(), set.size());
			for (const auto& element : expectedElements) {
				auto pElementFromSet = set.find(ToKey(element));
				EXPECT_EQ(*BaseType::ToPointer(element), *pElementFromSet);
			}
		}

		static auto ToKey(const ElementType& element) {
			return TBaseSetTraits::SetTraits::ToKey(element);
		}

		static auto ToStorage(const typename TBaseSetTraits::SetTraits::ValueType& value) {
			return TBaseSetTraits::SetTraits::ToStorage(value);
		}

		static auto CreateKey(const std::string& name, unsigned int value) {
			return ToKey(BaseType::CreateElement(name, value));
		}

		static auto ToPointerFromStorage(const typename TBaseSetTraits::SetTraits::StorageType& storage) {
			return BaseType::ToPointer(TBaseSetTraits::SetTraits::ToValue(storage));
		}

		static bool IsElementMutable() {
			return IsMutable<ElementType>();
		}
	};

	// endregion

	// region Base/Delta traits

	/// Traits for creating base sets.
	template<typename TBaseSetTraits>
	struct BaseTraits : BatchElementFactory<TBaseSetTraits> {
		static constexpr auto Commit = TBaseSetTraits::Commit;

		template<typename... TArgs>
		static auto Create(TArgs&&... args) {
			return TBaseSetTraits::Create(std::forward<TArgs>(args)...);
		}

		static auto CreateWithElements(size_t count) {
			auto pBaseSet = Create();
			auto pDelta = pBaseSet->rebase();
			BatchElementFactory<TBaseSetTraits>::InsertElements(*pDelta, count);
			Commit(*pBaseSet);
			return pBaseSet;
		}
	};

	/// Traits for creating deltas.
	template<typename TBaseSetTraits>
	struct DeltaTraits : BatchElementFactory<TBaseSetTraits> {
		template<typename... TArgs>
		static auto Create(TArgs&&... args) {
			auto pBaseSet = TBaseSetTraits::Create(std::forward<TArgs>(args)...);
			return ContainerDeltaPair<TBaseSetTraits>(pBaseSet, pBaseSet->rebase());
		}

		static auto CreateWithElements(size_t count) {
			auto pBaseSet = CreateBase();
			auto pDelta = pBaseSet->rebase();
			BatchElementFactory<TBaseSetTraits>::InsertElements(*pDelta, count);
			return ContainerDeltaPair<TBaseSetTraits>(pBaseSet, pDelta);
		}

		static auto CreateBase() {
			return TBaseSetTraits::Create();
		}
	};

	// endregion

	// region type declarations for common set types

	struct MapKeyHasher {
		size_t operator()(const std::pair<std::string, unsigned int>& pair) const {
			std::hash<std::string> stringHasher;
			std::hash<unsigned int> intHasher;
			return stringHasher(pair.first) ^ intHasher(pair.second);
		}
	};

	template<typename T>
	struct TestElementToKeyConverter {
		static auto ToKey(const T& element) {
			return std::make_pair(element.Name, element.Value);
		}
	};

	template<typename T>
	struct TestElementToKeyConverter<std::shared_ptr<T>> {
		static auto ToKey(const std::shared_ptr<T>& pElement) {
			return std::make_pair(pElement->Name, pElement->Value);
		}
	};

	template<typename TElement>
	using OrderedSetTraits = deltaset::SetStorageTraits<std::set<TElement, deltaset::detail::OrderedSetDefaultComparator<TElement>>>;

	template<typename TElement>
	using UnorderedSetTraits = deltaset::SetStorageTraits<std::unordered_set<TElement, Hasher<TElement>, EqualityChecker<TElement>>>;

	template<typename TElement>
	using ReverseOrderedSetTraits = deltaset::SetStorageTraits<std::set<TElement, ReverseComparator<TElement>>>;

	template<typename TElement>
	using UnorderedMapSetTraits = deltaset::MapStorageTraits<
		std::unordered_map<std::pair<std::string, unsigned int>, TElement, MapKeyHasher>,
		TestElementToKeyConverter<TElement>>;

	template<typename TMutabilityTraits>
	using SetElementType = typename std::remove_const<typename TMutabilityTraits::ElementType>::type;

	template<typename TElementTraits, typename TSetTraits>
	struct BaseSetTraits {
		using Type = deltaset::BaseSet<TElementTraits, TSetTraits>;
		using DeltaType = deltaset::BaseSetDelta<TElementTraits, TSetTraits>;
		using ElementType = typename TElementTraits::ElementType;
		using SetTraits = TSetTraits;

		template<typename... TArgs>
		static auto Create(TArgs&&... args) {
			return std::make_shared<Type>(std::forward<TArgs>(args)...);
		}

		static void Commit(Type& set) {
			set.commit();
		}
	};

	using MutableElementValueTraits = deltaset::MutableTypeTraits<MutableTestElement>;
	using MutableElementPointerTraits = deltaset::MutableTypeTraits<std::shared_ptr<MutableTestElement>>;
	using ImmutableElementValueTraits = deltaset::ImmutableTypeTraits<const ImmutableTestElement>;
	using ImmutablePointerValueTraits = deltaset::ImmutableTypeTraits<std::shared_ptr<const ImmutableTestElement>>;

	// endregion

	// region other utils

	/// Dereferences pointer \a pElement.
	template<typename T>
	T Unwrap(T* pElement) {
		return *pElement;
	}

	template<typename T>
	T Unwrap(const std::shared_ptr<T>& pElement) {
		return *pElement;
	}

	/// Returns \c true if container allows native value modification.
	template<typename T>
	bool AllowsNativeValueModification(const T&) {
		return IsMap(typename T::SetType());
	}

	// endregion

	// region asserts

	/// Asserts that the iterator exposed by \a set points to a const element.
	template<typename TBaseSet>
	void AssertConstIterator(const TBaseSet& set) {
		auto iterableSet = MakeIterableView(set);
		auto iter = iterableSet.begin();

		// Assert: the unwrapped set iterator element pointer points to a const element
		EXPECT_TRUE(std::is_const<decltype(Unwrap(iter.operator->()))>());
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
		CATAPULT_LOG(debug)
				<< "size: " << deltaWrapper.originalSize()
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
		CATAPULT_LOG(debug)
				<< "size: " << set.size()
				<< " (O " << set.size()
				<< ", A " << deltas.Added.size()
				<< ", R " << deltas.Removed.size()
				<< ", C " << deltas.Copied.size() << ")";
		EXPECT_EQ(expectedOriginal, set.size());
		EXPECT_EQ(expectedAdded, deltas.Added.size());
		EXPECT_EQ(expectedRemoved, deltas.Removed.size());
		EXPECT_EQ(expectedCopied, deltas.Copied.size());
	}

	/// Utilities when testing base set deltas.
	template<typename TTraits>
	struct BaseSetDeltaTestUtils {
		/// Sets the dummy value of the element with \a value in \a set to \a dummy.
		static void SetDummyValue(const decltype(*TTraits::Create())& set, unsigned int value, size_t dummy) {
			// Act: find the matching element and update its dummy value
			auto pOriginalElement = set.find(TTraits::CreateKey("TestElement", value));
			pOriginalElement->Dummy = dummy;
		}

		/// Creates a set with all types of elements for batch find tests.
		static auto CreateSetForBatchFindTests() {
			auto pDelta = TTraits::CreateWithElements(4);
			pDelta.commit();
			pDelta->emplace("TestElement", static_cast<unsigned int>(7));
			pDelta->emplace("TestElement", static_cast<unsigned int>(5));
			pDelta->emplace("TestElement", static_cast<unsigned int>(4));
			pDelta->remove(TTraits::CreateKey("TestElement", 1));
			pDelta->remove(TTraits::CreateKey("TestElement", 4));
			SetDummyValue(*pDelta, 2, 42);
			SetDummyValue(*pDelta, 5, 42);
			return pDelta;
		}

		/// Creates a set containing the expected elements in the base set delta created by CreateSetForBatchFindTests.
		static std::set<TestElement> CreateExpectedElementsForBatchFindTests() {
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
	};

	// endregion
}}
