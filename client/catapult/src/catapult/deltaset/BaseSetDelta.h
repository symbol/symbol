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

	/// A delta on top of a base set.
	/// \tparam TElementTraits Traits describing the type of element.
	/// \tparam TSetTraits Traits describing the underlying set.
	///
	/// The delta offers methods to insert/remove/update elements.
	/// \note This class is not thread safe.
	template<typename TElementTraits, typename TSetTraits>
	class BaseSetDelta : public utils::NonCopyable {
	public:
		using ElementType = typename TElementTraits::ElementType;
		using SetType = typename TSetTraits::SetType;
		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = detail::FindTraits<ElementType, TSetTraits::AllowsNativeValueModification>;
		using SetTraits = TSetTraits;

	public:
		/// Creates a delta on top of \a originalElements.
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
			if (nullptr != pOriginal)
				return pOriginal;

			auto addedIter = set.m_addedElements.find(key);
			return set.m_addedElements.cend() != addedIter ? ToResult(*addedIter) : nullptr;
		}

		typename FindTraits::ResultType find(const KeyType& key, MutableTypeTag) {
			auto copiedIter = m_copiedElements.find(key);
			if (m_copiedElements.cend() != copiedIter)
				return ToResult(*copiedIter);

			auto pOriginal = find(key, ImmutableTypeTag());
			if (nullptr == pOriginal)
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
		static constexpr bool contains(const SetType& set, const KeyType& key) {
			return set.cend() != set.find(key);
		}

	public:
		/// Inserts \a element into this set.
		/// \note The algorithm relies on the data used for comparing elements being immutable.
		InsertResult insert(const ElementType& element) {
			return insert(element, typename TElementTraits::MutabilityTag());
		}

		/// Creates an element around the passed arguments (\a args) and inserts the element into this set.
		template<typename... TArgs>
		InsertResult emplace(TArgs&&... args) {
			auto element = detail::ElementCreator<ElementType>::Create(std::forward<TArgs>(args)...);
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
		/// The actual iterator
		class iterator {
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = const typename TSetTraits::StorageType;
			using pointer = value_type*;
			using reference = value_type&;
			using iterator_category = std::forward_iterator_tag;

		public:
			/// Creates an iterator around the original \a elements and \a deltas at \a position.
			iterator(const SetType& elements, const DeltaElements<SetType>& deltas, size_t position)
					: m_elements(elements)
					, m_deltas(deltas)
					, m_position(position)
					, m_size(m_elements.size() + m_deltas.Added.size() - m_deltas.Removed.size())
					, m_stage(IterationStage::Copied)
					, m_iter(m_deltas.Copied.cbegin()) {
				if (m_position == m_size)
					m_iter = m_deltas.Added.cend();
				else
					moveToValidElement();
			}

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const iterator& rhs) const {
				return &m_elements == &rhs.m_elements && m_position == rhs.m_position;
			}

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const iterator& rhs) const {
				return !(*this == rhs);
			}

		public:
			/// Advances the iterator to the next position.
			iterator& operator++() {
				if (m_position == m_size)
					CATAPULT_THROW_OUT_OF_RANGE("cannot advance iterator beyond end");

				++m_position;
				++m_iter;
				moveToValidElement();
				return *this;
			}

			/// Advances the iterator to the next position.
			iterator operator++(int) {
				auto copy = *this;
				++*this;
				return copy;
			}

		private:
			void moveToValidElement() {
				if (IterationStage::Copied == m_stage) {
					if (handleCopiedStage())
						return;

					// all copied elements have been iterated, so advance to the next stage
					m_stage = IterationStage::Original;
					m_iter = m_elements.cbegin();
				}

				if (IterationStage::Original == m_stage) {
					if (handleOriginalStage())
						return;

					// all original elements have been iterated, so advance to the next stage
					m_stage = IterationStage::Added;
					m_iter = m_deltas.Added.cbegin();
				}

				// last stage, nothing left to do
			}

			bool handleOriginalStage() {
				// advance to the first original element that has neither been removed nor copied
				for (; m_elements.end() != m_iter; ++m_iter) {
					const auto& key = TSetTraits::ToKey(*m_iter);
					if (!contains(m_deltas.Removed, key) && !contains(m_deltas.Copied, key))
						return true;
				}

				return false;
			}

			bool handleCopiedStage() {
				return m_deltas.Copied.end() != m_iter;
			}

			static CPP14_CONSTEXPR bool contains(const SetType& set, const KeyType& key) {
				return set.cend() != set.find(key);
			}

		public:
			/// Returns a pointer to the current element.
			value_type* operator->() const {
				if (m_position == m_size)
					CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

				return &*m_iter;
			}

			/// Returns a reference to the current element.
			value_type& operator*() const {
				return *(this->operator->());
			}

		private:
			enum class IterationStage { Copied, Original, Added };

		private:
			const SetType& m_elements;
			DeltaElements<SetType> m_deltas;
			size_t m_position;
			size_t m_size;
			IterationStage m_stage;
			typename SetType::const_iterator m_iter;
		};

		/// Returns a const iterator to the first element of the underlying set.
		auto begin() const {
			return iterator(m_originalElements, deltas(), 0);
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto end() const {
			return iterator(m_originalElements, deltas(), size());
		}

	public:
		/// Gets const references to the pending modifications.
		DeltaElements<SetType> deltas() const {
			return DeltaElements<SetType>(m_addedElements, m_removedElements, m_copiedElements);
		}

		/// Resets all pending modifications.
		void reset() {
			m_addedElements.clear();
			m_removedElements.clear();
			m_copiedElements.clear();
		}

	private:
		const SetType& m_originalElements;
		SetType m_addedElements;
		SetType m_removedElements;
		SetType m_copiedElements;
	};
}}
