#pragma once
#include "ContainerDeltaPair.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/BaseSetDefaultTraits.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "tests/TestHarness.h"
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace test {

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

	// region Hasher

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

	// endregion

	// region ReverseComparator

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

	// region IsMutable

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
			EXPECT_EQ(expectedElements.size(), set.size());
			for (const auto& element : expectedElements) {
				auto pElementFromSet = set.find(ToKey(element));
				EXPECT_EQ(*BaseType::ToPointer(element), *pElementFromSet);
			}
		}

		static auto ToKey(const ElementType& element) {
			return TBaseSetTraits::SetTraits::ToKey(element);
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

	/// Traits for creating base sets.
	template<typename TBaseSetTraits>
	struct BaseTraits : BatchElementFactory<TBaseSetTraits> {

		static auto Create() {
			return TBaseSetTraits::Create();
		}

		static auto CreateWithElements(size_t count) {
			auto pBaseSet = Create();
			auto pDelta = pBaseSet->rebase();
			BatchElementFactory<TBaseSetTraits>::InsertElements(*pDelta, count);
			TBaseSetTraits::Commit(*pBaseSet);
			return pBaseSet;
		}
	};

	/// Traits for creating deltas.
	template<typename TBaseSetTraits>
	struct DeltaTraits : BatchElementFactory<TBaseSetTraits> {

		static auto Create() {
			auto pBaseSet = CreateBase();
			return test::ContainerDeltaPair<TBaseSetTraits>(pBaseSet, pBaseSet->rebase());
		}

		static auto CreateWithElements(size_t count) {
			auto pBaseSet = CreateBase();
			auto pDelta = pBaseSet->rebase();
			BatchElementFactory<TBaseSetTraits>::InsertElements(*pDelta, count);
			return test::ContainerDeltaPair<TBaseSetTraits>(pBaseSet, pDelta);
		}

		static auto CreateBase() {
			return TBaseSetTraits::Create();
		}
	};

	template<typename TElement>
	using OrderedSetTraits = deltaset::SetStorageTraits<std::set<TElement, deltaset::DefaultComparator<TElement>>>;

	template<typename TElement>
	using UnorderedSetTraits =
		deltaset::SetStorageTraits<std::unordered_set<TElement, Hasher<TElement>, deltaset::EqualityChecker<TElement>>>;

	template<typename TElement>
	using ReverseOrderedSetTraits = deltaset::SetStorageTraits<std::set<TElement, ReverseComparator<TElement>>>;

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
			const auto& elementRef = deltaset::detail::DerefHelper<T>::Deref(element);
			return std::make_pair(elementRef.Name, elementRef.Value);
		}
	};

	template<typename T>
	using UnorderedMapSetTraits = deltaset::MapStorageTraits<
			std::unordered_map<std::pair<std::string, unsigned int>, T, MapKeyHasher>,
			TestElementToKeyConverter<T>>;

	template<typename TMutabilityTraits>
	using SetElementType = typename std::remove_const<typename TMutabilityTraits::ElementType>::type;

	template<typename TElementTraits, typename TSetTraits>
	struct BaseSetTraits {
		using Type = deltaset::BaseSet<TElementTraits, TSetTraits>;
		using DeltaType = deltaset::BaseSetDelta<TElementTraits, TSetTraits>;
		using ElementType = typename TElementTraits::ElementType;
		using SetTraits = TSetTraits;

		static auto Create() {
			return std::make_shared<Type>();
		}

		static void Commit(Type& set) {
			set.commit();
		}
	};

	template<typename TMutabilityTraits>
	using OrderedTraits = BaseSetTraits<
			TMutabilityTraits,
			OrderedSetTraits<SetElementType<TMutabilityTraits>>>;

	template<typename TMutabilityTraits>
	using UnorderedTraits = BaseSetTraits<
			TMutabilityTraits,
			UnorderedSetTraits<SetElementType<TMutabilityTraits>>>;

	template<typename TMutabilityTraits>
	using ReverseOrderedTraits = BaseSetTraits<
			TMutabilityTraits,
			ReverseOrderedSetTraits<SetElementType<TMutabilityTraits>>>;

	template<typename TMutabilityTraits>
	using UnorderedMapTraits = BaseSetTraits<
			TMutabilityTraits,
			UnorderedMapSetTraits<SetElementType<TMutabilityTraits>>>;
}}
