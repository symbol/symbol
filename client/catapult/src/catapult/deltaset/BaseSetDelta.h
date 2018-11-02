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
#include "BaseSetFindIterator.h"
#include "DeltaElements.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include <memory>
#include <type_traits>
#include <unordered_map>

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
		using ElementMutabilityTag = typename TElementTraits::MutabilityTag;

		using SetType = typename TSetTraits::SetType;
		using MemorySetType = typename TSetTraits::MemorySetType;

		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = FindTraitsT<ElementType, TSetTraits::AllowsNativeValueModification>;
		using SetTraits = TSetTraits;

		using FindIterator = typename std::conditional<
			std::is_same<ElementMutabilityTag, ImmutableTypeTag>::value,
			BaseSetDeltaFindConstIterator<FindTraits, TSetTraits>,
			BaseSetDeltaFindIterator<FindTraits, TSetTraits>
		>::type;
		using FindConstIterator = BaseSetDeltaFindConstIterator<FindTraits, TSetTraits>;

	public:
		/// Creates a delta around \a originalElements.
		explicit BaseSetDelta(const SetType& originalElements)
				: m_originalElements(originalElements)
				, m_generationId(1)
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
		FindConstIterator find(const KeyType& key) const {
			return find<FindConstIterator>(*this, key);
		}

		/// Searches for \a key in this set.
		/// Returns a pointer to the matching element if it is found or \c nullptr if it is not found.
		FindIterator find(const KeyType& key) {
			auto iter = find<FindIterator>(*this, key);
			if (!!iter.get())
				markFoundElement(key, ElementMutabilityTag());

			return iter;
		}

	private:
		template<typename TResultIterator, typename TBaseSetDelta>
		static TResultIterator find(TBaseSetDelta& set, const KeyType& key) {
			if (contains(set.m_removedElements, key))
				return TResultIterator();

			auto originalIter = set.find(key, ElementMutabilityTag());
			if (originalIter.get())
				return originalIter;

			auto addedIter = set.m_addedElements.find(key);
			return set.m_addedElements.cend() != addedIter ? TResultIterator(std::move(addedIter)) : TResultIterator();
		}

		FindIterator find(const KeyType& key, MutableTypeTag) {
			auto copiedIter = m_copiedElements.find(key);
			if (m_copiedElements.cend() != copiedIter)
				return FindIterator(std::move(copiedIter));

			auto originalIter = find(key, ImmutableTypeTag());
			if (!originalIter.get())
				return FindIterator();

			auto copy = TElementTraits::Copy(originalIter.get());
			auto result = m_copiedElements.insert(SetTraits::ToStorage(copy));
			return FindIterator(std::move(result.first));
		}

		FindConstIterator find(const KeyType& key, MutableTypeTag) const {
			auto copiedIter = m_copiedElements.find(key);
			return m_copiedElements.cend() != copiedIter
					? FindConstIterator(std::move(copiedIter))
					: find(key, ImmutableTypeTag());
		}

		FindConstIterator find(const KeyType& key, ImmutableTypeTag) const {
			auto originalIter = m_originalElements.find(key);
			return m_originalElements.cend() != originalIter ? FindConstIterator(std::move(originalIter)) : FindConstIterator();
		}

		void markFoundElement(const KeyType& key, MutableTypeTag) {
			markKey(key);
		}

		void markFoundElement(const KeyType&, ImmutableTypeTag) {
			// find can never modify an immutable element
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
			return insert(element, ElementMutabilityTag());
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
				markKey(key);
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
			markKey(key);
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
				clearKey(key);
				m_removedElements.erase(removedIter);
				return InsertResult::Unremoved;
			}

			if (contains(m_originalElements, key) || contains(m_addedElements, key))
				return InsertResult::Redundant;

			markKey(key);
			m_addedElements.insert(TSetTraits::ToStorage(element));
			return InsertResult::Inserted;
		}

	public:
		/// Removes the element identified by \a key from the delta.
		RemoveResult remove(const KeyType& key) {
			if (contains(m_removedElements, key))
				return RemoveResult::Redundant;

			return remove(key, ElementMutabilityTag());
		}

	private:
		RemoveResult remove(const KeyType& key, MutableTypeTag) {
			auto copiedIter = m_copiedElements.find(key);
			if (m_copiedElements.cend() != copiedIter) {
				markKey(key);
				m_removedElements.insert(*copiedIter);
				m_copiedElements.erase(copiedIter);
				return RemoveResult::Unmodified_And_Removed;
			}

			return remove(key, ImmutableTypeTag());
		}

		RemoveResult remove(const KeyType& key, ImmutableTypeTag) {
			auto addedIter = m_addedElements.find(key);
			if (m_addedElements.cend() != addedIter) {
				clearKey(key);
				m_addedElements.erase(addedIter);
				return RemoveResult::Uninserted;
			}

			auto originalIter = m_originalElements.find(key);
			if (m_originalElements.cend() != originalIter) {
				markKey(key);
				m_removedElements.insert(TSetTraits::ToStorage(key, std::move(originalIter)));
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

			m_generationId = 1;
			m_keyGenerationIdMap.clear();
		}

	public:
		/// Gets the current generation id.
		uint32_t generationId() const {
			return m_generationId;
		}

		/// Gets the generation id associated with \a key.
		uint32_t generationId(const KeyType& key) const {
			auto iter = m_keyGenerationIdMap.find(key);
			return m_keyGenerationIdMap.cend() == iter ? 0 : iter->second;
		}

		/// Increments the generation id.
		void incrementGenerationId() {
			++m_generationId;
		}

	private:
		void markKey(const KeyType& key) {
			// latest generation id should always be stored
			m_keyGenerationIdMap[key] = m_generationId;
		}

		void clearKey(const KeyType& key) {
			m_keyGenerationIdMap.erase(key);
		}

	private:
		// for sorted containers, use map because no hasher is specified
		template<typename T, typename = void>
		struct KeyGenerationIdMap {
			using type = std::map<KeyType, uint32_t, typename T::key_compare>;
		};

		// for hashed containers, use unordered_map because hasher is specified
		template<typename T>
		struct KeyGenerationIdMap<T, typename utils::traits::enable_if_type<typename T::hasher>::type> {
			using type = std::unordered_map<KeyType, uint32_t, typename T::hasher, typename T::key_equal>;
		};

	private:
		const SetType& m_originalElements;
		MemorySetType m_addedElements;
		MemorySetType m_removedElements;
		MemorySetType m_copiedElements;

		uint32_t m_generationId;
		typename KeyGenerationIdMap<SetType>::type m_keyGenerationIdMap;

	private:
		template<typename TElementTraits2, typename TSetTraits2>
		friend BaseSetDeltaIterationView<TSetTraits2> MakeIterableView(const BaseSetDelta<TElementTraits2, TSetTraits2>& set);
	};
}}
