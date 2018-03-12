#pragma once
#include <memory>

namespace catapult { namespace deltaset {

	// region DefaultComparator

	/// Compares two objects of the same type.
	template<typename T>
	struct DefaultComparator {
		bool operator()(const T& lhs, const T& rhs) const {
			return lhs < rhs;
		}
	};

	template<typename T>
	struct DefaultComparator<std::shared_ptr<T>> {
		bool operator()(const std::shared_ptr<T>& pLhs, const std::shared_ptr<T>& pRhs) const {
			DefaultComparator<T> comparator;
			return comparator(*pLhs, *pRhs);
		}
	};

	// endregion

	// region EqualityChecker

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

	// endregion

	// region storage traits

	/// Base set compatible traits for stl set types.
	template<typename TSet>
	struct SetStorageTraits {
		using SetType = TSet;
		using KeyType = typename TSet::value_type;
		using ValueType = typename TSet::value_type;
		using StorageType = typename TSet::value_type;

		/// Set values cannot be modified because they are hashed in native container.
		static constexpr bool AllowsNativeValueModification = false;

		/// Converts a value type (\a element) to a key.
		static constexpr const KeyType& ToKey(const ValueType& element) {
			return element;
		}

		/// Converts a value type (\a element) to a storage type.
		static constexpr const StorageType& ToStorage(const ValueType& element) {
			return element;
		}

		/// Converts a storage type (\a element) to a value type.
		static constexpr const ValueType& ToValue(const StorageType& element) {
			return element;
		}
	};

	namespace detail {
		template<typename TElement>
		struct DerefHelper {
			using const_pointer_type = const TElement*;

			static const TElement& Deref(const TElement& element) {
				return element;
			}
		};

		template<typename T>
		struct DerefHelper<std::shared_ptr<T>> {
			using const_pointer_type = const T*;

			static const T& Deref(const std::shared_ptr<T>& element) {
				return *element;
			}
		};
	}

	/// Base set compatible traits for stl map types.
	template<typename TMap, typename TElementToKeyConverter>
	struct MapStorageTraits {
		using SetType = TMap;
		using KeyType = typename TMap::key_type;
		using ValueType = typename TMap::mapped_type;
		using StorageType = typename TMap::value_type;

		/// Map values can be modified because they are not hashed in native container.
		static constexpr bool AllowsNativeValueModification = true;

		/// Converts a value type (\a element) to a key.
		static constexpr KeyType ToKey(const ValueType& element) {
			return TElementToKeyConverter::ToKey(element);
		}

		/// Converts a storage type (\a element) to a key.
		static constexpr const KeyType& ToKey(const StorageType& element) {
			return element.first;
		}

		/// Converts a value type (\a element) to a storage type.
		static constexpr StorageType ToStorage(const ValueType& element) {
			return std::make_pair(ToKey(element), element);
		}

		/// Converts a storage type (\a element) to a value type.
		static constexpr const ValueType& ToValue(const StorageType& element) {
			return element.second;
		}

		/// Converts a storage type (\a element) to a value type.
		static constexpr ValueType& ToValue(StorageType& element) {
			return element.second;
		}
	};

	// endregion

	// region MutableTypeTag / MutableTypeTraits

	/// Tag that indicates a type is mutable.
	struct MutableTypeTag {};

	namespace detail {
		template<typename T>
		struct ElementDeepCopy {
			static constexpr T Copy(const T* pElement) {
				return *pElement;
			}
		};

		template<typename T>
		struct ElementDeepCopy<std::shared_ptr<T>> {
			static std::shared_ptr<T> Copy(const std::shared_ptr<const T>& pElement) {
				return std::make_shared<T>(*pElement);
			}
		};
	}

	/// Traits used for describing a mutable type.
	template<typename TElement>
	struct MutableTypeTraits : public detail::ElementDeepCopy<TElement> {
		using ElementType = TElement;
		using MutabilityTag = MutableTypeTag;
	};

	// endregion

	// region ImmutableTypeTag / ImmutableTypeTraits

	/// Tag that indicates a type is immutable.
	struct ImmutableTypeTag {};

	/// Traits used for describing an immutable type.
	template<typename TElement>
	struct ImmutableTypeTraits {
		using ElementType = const TElement;
		using MutabilityTag = ImmutableTypeTag;
	};

	template<typename T>
	struct ImmutableTypeTraits<std::shared_ptr<T>> {
		using ElementType = std::shared_ptr<T>;
		using MutabilityTag = ImmutableTypeTag;
	};

	// endregion

	namespace detail {

		// region FindTraits

		/// Traits for customizing the behavior of find depending on element type.
		template<typename T, bool AllowsNativeValueModification>
		struct FindTraits {
			using ConstResultType = const T*;
			using ResultType = const T*;

			static constexpr ResultType ToResult(const T& value) {
				return &value;
			}
		};

		template<typename T>
		struct FindTraits<T, true> {
			using ConstResultType = const T*;
			using ResultType = T*;

			// this needs to be a template in order to allow T to be const (immutable)
			template<typename TValue>
			static constexpr auto ToResult(TValue& value) {
				return &value;
			}
		};

		// the object pointed to by shared_ptr can be modified in any type of container
		template<typename T>
		struct SharedPtrFindTraits {
			using ConstResultType = std::shared_ptr<const T>;
			using ResultType = std::shared_ptr<T>;

			static constexpr ResultType ToResult(const std::shared_ptr<T>& value) {
				return value;
			}
		};

		template<typename T>
		struct FindTraits<std::shared_ptr<T>, true> : public SharedPtrFindTraits<T>
		{};

		template<typename T>
		struct FindTraits<std::shared_ptr<T>, false> : public SharedPtrFindTraits<T>
		{};

		// endregion

		// region ElementCreator

		/// Traits for creating an element from arguments.
		template<typename T>
		struct ElementCreator {
			template<typename... TArgs>
			static T Create(TArgs&&... args) {
				return T(std::forward<TArgs>(args)...);
			}
		};

		template<typename T>
		struct ElementCreator<std::shared_ptr<T>> {
			template<typename... TArgs>
			static std::shared_ptr<T> Create(TArgs&&... args) {
				return std::make_shared<T>(std::forward<TArgs>(args)...);
			}
		};

		// endregion
	}
}}
