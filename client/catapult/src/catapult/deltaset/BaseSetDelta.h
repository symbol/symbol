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
#include "BaseSetDefaultTraits.h"
#include "DeltaElements.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include <memory>

namespace catapult { namespace deltaset {

	/// Possible results of an insert into a base set delta.
	enum class InsertResult {
		/// An element pending removal was reverted.
		Unremoved,
		/// An existing element was updated (mutable elements only).
		Updated,
		/// Insert failed because the element already exists (immutable elements only).
		Redundant,
		/// A new element was inserted.
		Inserted
	};

	/// Possible results of a remove from a base set delta.
	enum class RemoveResult {
		/// No matching element was found.
		None,
		/// An element pending modification was reverted and removed (mutable elements only).
		Unmodified_And_Removed,
		/// An element pending insert was reverted.
		Uninserted,
		/// Remove failed because the element already was removed.
		Redundant,
		/// An existing element was removed.
		Removed
	};

	template<typename TSetTraits>
	class BaseSetDeltaIterationView;

	/// A delta on top of a base set that offers methods to insert/remove/update elements.
	/// \tparam TElementTraits Traits describing the type of element.
	/// \tparam TSetTraits Traits describing the underlying set.
	///
	/// \note This class is not thread safe.
	template<typename TElementTraits, typename TSetTraits>
	class BaseSetDelta : public utils::NonCopyable {
	public:
		using ElementType = typename TElementTraits::ElementType;

		using SetType = typename TSetTraits::SetType;
		using MemorySetType = typename TSetTraits::MemorySetType;

		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = FindTraitsT<ElementType, TSetTraits::AllowsNativeValueModification>;
		using SetTraits = TSetTraits;

	public:
		/// Creates a delta around \a originalElements.
		explicit BaseSetDelta(const SetType& originalElements) : m_originalElements(originalElements)
		{}

	public:
		/// Gets a value indicating whether or not the set is empty.
		bool empty() const {
			return 0 == size();
		}

		/// Gets the size of this set.
		size_t size() const {
			return m_originalElements.size() - m_removedElements.size() + m_addedElements.size();
		}

	public:
		/// Searches for \a key in this set.
		/// Returns a pointer to the matching element if it is found or \c nullptr if it is not found.
		typename FindTraits::ConstResultType find(const KeyType& key) const {
			return find<decltype(*this), typename FindTraits::ConstResultType>(*this, key);
		}

		/// Searches for \a key in this set.
		/// Returns a pointer to the matching element if it is found or \c nullptr if it is not found.
		typename FindTraits::ResultType find(const KeyType& key) {
			return find<decltype(*this), typename FindTraits::ResultType>(*this, key);
		}

	private:
		template<typename TStorageType>
		static constexpr auto ToResult(TStorageType& storage) {
			return FindTraits::ToResult(TSetTraits::ToValue(storage));
		}

		template<typename TBaseSetDelta, typename TResult>
		static TResult find(TBaseSetDelta& set, const KeyType& key) {
			if (contains(set.m_removedElements, key))
				return nullptr;

			auto pOriginal = set.find(key, typename TElementTraits::MutabilityTag());
			if (pOriginal)
				return pOriginal;

			auto addedIter = set.m_addedElements.find(key);
			return set.m_addedElements.cend() != addedIter ? ToResult(*addedIter) : nullptr;
		}

		typename FindTraits::ResultType find(const KeyType& key, MutableTypeTag) {
			auto copiedIter = m_copiedElements.find(key);
			if (m_copiedElements.cend() != copiedIter)
				return ToResult(*copiedIter);

			auto pOriginal = find(key, ImmutableTypeTag());
			if (!pOriginal)
				return nullptr;

			auto copy = TElementTraits::Copy(pOriginal);
			auto result = m_copiedElements.insert(SetTraits::ToStorage(copy));
			return ToResult(*result.first);
		}

		typename FindTraits::ConstResultType find(const KeyType& key, MutableTypeTag) const {
			auto copiedIter = m_copiedElements.find(key);
			return m_copiedElements.cend() != copiedIter
					? ToResult(*copiedIter)
					: find(key, ImmutableTypeTag());
		}

		typename FindTraits::ConstResultType find(const KeyType& key, ImmutableTypeTag) const {
			auto originalIter = m_originalElements.find(key);
			return m_originalElements.cend() != originalIter ? ToResult(*originalIter) : nullptr;
		}

	public:
		/// Searches for \a key in this set.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const KeyType& key) const {
			return !contains(m_removedElements, key) && (contains(m_addedElements, key) || contains(m_originalElements, key));
		}

	private:
		template<typename TSet> // SetType or MemorySetType
		static constexpr bool contains(const TSet& set, const KeyType& key) {
			return set.cend() != set.find(key);
		}

	private:
		// used to support creating values and values pointed to by shared_ptr
		// (this is required to support shared_ptr value types in BaseSet)

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

	public:
		/// Inserts \a element into this set.
		/// \note The algorithm relies on the data used for comparing elements being immutable.
		InsertResult insert(const ElementType& element) {
			return insert(element, typename TElementTraits::MutabilityTag());
		}

		/// Creates an element around the passed arguments (\a args) and inserts the element into this set.
		template<typename... TArgs>
		InsertResult emplace(TArgs&&... args) {
			auto element = ElementCreator<ElementType>::Create(std::forward<TArgs>(args)...);
			return insert(element);
		}

	private:
		InsertResult insert(const ElementType& element, MutableTypeTag) {
			const auto& key = TSetTraits::ToKey(element);
			auto removedIter = m_removedElements.find(key);
			if (m_removedElements.cend() != removedIter) {
				// since the element is in the set of removed elements, it must be an original element
				// and cannot be in the set of added elements
				m_removedElements.erase(removedIter);

				// since the element is mutable, it could have been modified, so add it to the copied elements
				m_copiedElements.insert(TSetTraits::ToStorage(element));
				return InsertResult::Unremoved;
			}

			auto insertResult = InsertResult::Inserted;
			decltype(m_copiedElements)* pTargetElements;
			if (contains(m_originalElements, key)) {
				pTargetElements = &m_copiedElements; // original element, possibly modified
				insertResult = InsertResult::Updated;
			} else {
				pTargetElements = &m_addedElements; // not an original element
				if (contains(m_addedElements, key))
					insertResult = InsertResult::Updated;
			}

			// copy the storage before erasing in case element is sourced from the same container being updated
			auto storage = TSetTraits::ToStorage(element);
			pTargetElements->erase(key);
			pTargetElements->insert(std::move(storage));
			return insertResult;
		}

		InsertResult insert(const ElementType& element, ImmutableTypeTag) {
			const auto& key = TSetTraits::ToKey(element);
			auto removedIter = m_removedElements.find(key);
			if (m_removedElements.cend() != removedIter) {
				// since the element is in the set of removed elements, it must be an original element
				// and cannot be in the set of added elements
				m_removedElements.erase(removedIter);
				return InsertResult::Unremoved;
			}

			if (contains(m_originalElements, key) || contains(m_addedElements, key))
				return InsertResult::Redundant;

			m_addedElements.insert(TSetTraits::ToStorage(element));
			return InsertResult::Inserted;
		}

	public:
		/// Removes the element identified by \a key from the delta.
		RemoveResult remove(const KeyType& key) {
			if (contains(m_removedElements, key))
				return RemoveResult::Redundant;

			return remove(key, typename TElementTraits::MutabilityTag());
		}

	private:
		RemoveResult remove(const KeyType& key, MutableTypeTag) {
			auto copiedIter = m_copiedElements.find(key);
			if (m_copiedElements.cend() != copiedIter) {
				m_removedElements.insert(*copiedIter);
				m_copiedElements.erase(copiedIter);
				return RemoveResult::Unmodified_And_Removed;
			}

			return remove(key, ImmutableTypeTag());
		}

		RemoveResult remove(const KeyType& key, ImmutableTypeTag) {
			auto addedIter = m_addedElements.find(key);
			if (m_addedElements.cend() != addedIter) {
				m_addedElements.erase(addedIter);
				return RemoveResult::Uninserted;
			}

			auto originalIter = m_originalElements.find(key);
			if (m_originalElements.cend() != originalIter) {
				m_removedElements.insert(*originalIter);
				return RemoveResult::Removed;
			}

			return RemoveResult::None;
		}

	public:
		/// Gets const references to the pending modifications.
		DeltaElements<MemorySetType> deltas() const {
			return DeltaElements<MemorySetType>(m_addedElements, m_removedElements, m_copiedElements);
		}

		/// Resets all pending modifications.
		void reset() {
			m_addedElements.clear();
			m_removedElements.clear();
			m_copiedElements.clear();
		}

	private:
		const SetType& m_originalElements;
		MemorySetType m_addedElements;
		MemorySetType m_removedElements;
		MemorySetType m_copiedElements;

	private:
		template<typename TElementTraits2, typename TSetTraits2>
		friend BaseSetDeltaIterationView<TSetTraits2> MakeIterableView(const BaseSetDelta<TElementTraits2, TSetTraits2>& set);
	};
}}
