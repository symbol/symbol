#pragma once
#include "BaseSetDefaultTraits.h"
#include "DeltaEntities.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include <memory>

namespace catapult { namespace deltaset {

	/// A delta on top of a base set.
	/// \tparam TEntityTraits Traits describing the type of entity.
	/// \tparam TSetTraits Traits describing the underlying set.
	///
	/// The delta offers methods to insert/remove/update elements.
	/// \note This class is not thread safe.
	template<typename TEntityTraits, typename TSetTraits>
	class BaseSetDelta : public utils::NonCopyable {
	public:
		using EntityType = typename TEntityTraits::EntityType;
		using SetType = typename TSetTraits::SetType;
		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = detail::FindTraits<EntityType, TSetTraits::AllowsNativeValueModification>;
		using SetTraits = TSetTraits;

	public:
		/// Creates a delta on top of \a originalEntities.
		explicit BaseSetDelta(const SetType& originalEntities) : m_originalEntities(originalEntities)
		{}

	public:
		/// Gets a value indicating whether or not the set is empty.
		bool empty() const {
			return 0 == size();
		}

		/// Gets the size of this set.
		size_t size() const {
			return m_originalEntities.size() - m_removedEntities.size() + m_addedEntities.size();
		}

	public:
		/// Searches for \a key in this set.
		/// Returns a pointer to the matching entity if it is found or \c nullptr if it is not found.
		typename FindTraits::ConstResultType find(const KeyType& key) const {
			return find<decltype(*this), typename FindTraits::ConstResultType>(*this, key);
		}

		/// Searches for \a key in this set.
		/// Returns a pointer to the matching entity if it is found or \c nullptr if it is not found.
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
			if (contains(set.m_removedEntities, key))
				return nullptr;

			auto pOriginal = set.find(key, typename TEntityTraits::MutabilityTag());
			if (nullptr != pOriginal)
				return pOriginal;

			auto addedIter = set.m_addedEntities.find(key);
			return set.m_addedEntities.cend() != addedIter ? ToResult(*addedIter) : nullptr;
		}

		typename FindTraits::ResultType find(const KeyType& key, MutableTypeTag) {
			auto copyIter = m_copiedEntities.find(key);
			if (m_copiedEntities.cend() != copyIter)
				return ToResult(*copyIter);

			auto pOriginal = find(key, ImmutableTypeTag());
			if (nullptr == pOriginal)
				return nullptr;

			auto copy = TEntityTraits::Copy(pOriginal);
			auto result = m_copiedEntities.insert(SetTraits::ToStorage(copy));
			return ToResult(*result.first);
		}

		typename FindTraits::ConstResultType find(const KeyType& key, MutableTypeTag) const {
			auto copyIter = m_copiedEntities.find(key);
			return m_copiedEntities.cend() != copyIter
					? ToResult(*copyIter)
					: find(key, ImmutableTypeTag());
		}

		typename FindTraits::ConstResultType find(const KeyType& key, ImmutableTypeTag) const {
			auto originalIter = m_originalEntities.find(key);
			return m_originalEntities.cend() != originalIter ? ToResult(*originalIter) : nullptr;
		}

	public:
		/// Searches for \a key in this set.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const KeyType& key) const {
			return !contains(m_removedEntities, key)
					&& (contains(m_addedEntities, key) || contains(m_originalEntities, key));
		}

	private:
		static constexpr bool contains(const SetType& set, const KeyType& key) {
			return set.cend() != set.find(key);
		}

	public:
		/// Inserts \a entity into this set.
		/// \note The algorithm relies on the data used for comparing elements being immutable.
		void insert(const EntityType& entity) {
			insert(entity, typename TEntityTraits::MutabilityTag());
		}

		/// Creates an entity around the passed arguments (\a args) and inserts the entity into this set.
		template<typename... TArgs>
		void emplace(TArgs&&... args) {
			auto entity = detail::EntityCreator<EntityType>::Create(std::forward<TArgs>(args)...);
			insert(entity);
		}

	private:
		void insert(const EntityType& entity, MutableTypeTag) {
			const auto& key = TSetTraits::ToKey(entity);
			auto removedIter = m_removedEntities.find(key);
			if (m_removedEntities.cend() != removedIter) {
				// since the entity is in the set of removed entities, it must be an original entity
				// and cannot be in the set of added entities
				m_removedEntities.erase(removedIter);

				// since the entity is mutable, it could have been modified, so add it to the copied entities
				m_copiedEntities.insert(TSetTraits::ToStorage(entity));
				return;
			}

			auto& targetContainer = contains(m_originalEntities, key)
					? m_copiedEntities // original entity, possibly modified
					: m_addedEntities; // not an original entity

			// copy the storage before erasing in case entity is sourced from the same container being updated
			auto storage = TSetTraits::ToStorage(entity);
			targetContainer.erase(key);
			targetContainer.insert(std::move(storage));
		}

		void insert(const EntityType& entity, ImmutableTypeTag) {
			const auto& key = TSetTraits::ToKey(entity);
			auto removedIter = m_removedEntities.find(key);
			if (m_removedEntities.cend() != removedIter) {
				// since the entity is in the set of removed entities, it must be an original entity
				// and cannot be in the set of added entities
				m_removedEntities.erase(removedIter);
				return;
			}

			if (contains(m_originalEntities, key))
				return;

			m_addedEntities.insert(TSetTraits::ToStorage(entity));
		}

	public:
		/// Removes the entity identified by \a key from the delta.
		void remove(const KeyType& key) {
			if (contains(m_removedEntities, key))
				return;

			remove(key, typename TEntityTraits::MutabilityTag());
		}

	private:
		void remove(const KeyType& key, MutableTypeTag) {
			auto copyIter = m_copiedEntities.find(key);
			if (m_copiedEntities.cend() != copyIter) {
				m_removedEntities.insert(*copyIter);
				m_copiedEntities.erase(copyIter);
				return;
			}

			remove(key, ImmutableTypeTag());
		}

		void remove(const KeyType& key, ImmutableTypeTag) {
			auto addedIter = m_addedEntities.find(key);
			if (m_addedEntities.cend() != addedIter) {
				m_addedEntities.erase(addedIter);
				return;
			}

			auto originalIter = m_originalEntities.find(key);
			if (m_originalEntities.cend() != originalIter) {
				m_removedEntities.insert(*originalIter);
				return;
			}
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
			/// Creates an iterator around the original \a entities and \a deltas at position \a position.
			iterator(
					const SetType& entities,
					const DeltaEntities<SetType>& deltas,
					size_t position)
					: m_entities(entities)
					, m_deltas(deltas)
					, m_position(position)
					, m_size(m_entities.size() + m_deltas.Added.size() - m_deltas.Removed.size())
					, m_stage(IterationStage::Copied)
					, m_iter(m_deltas.Copied.cbegin()) {
				if (m_position == m_size)
					m_iter = m_deltas.Added.cend();
				else
					moveToValidEntity();
			}

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const iterator& rhs) const {
				return &m_entities == &rhs.m_entities && m_position == rhs.m_position;
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
				moveToValidEntity();
				return *this;
			}

			/// Advances the iterator to the next position.
			iterator operator++(int) {
				auto copy = *this;
				++*this;
				return copy;
			}

		private:
			void moveToValidEntity() {
				if (IterationStage::Copied == m_stage) {
					if (handleCopiedStage())
						return;

					// all copied entities have been iterated, so advance to the next stage
					m_stage = IterationStage::Original;
					m_iter = m_entities.cbegin();
				}

				if (IterationStage::Original == m_stage) {
					if (handleOriginalStage())
						return;

					// all original entities have been iterated, so advance to the next stage
					m_stage = IterationStage::Added;
					m_iter = m_deltas.Added.cbegin();
				}

				// last stage, nothing left to do
			}

			bool handleOriginalStage() {
				// advance to the first original entity that has neither been removed nor copied
				for (; m_entities.end() != m_iter; ++m_iter) {
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
			/// Returns a pointer to the current entity.
			value_type* operator->() const {
				if (m_position == m_size)
					CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

				return &*m_iter;
			}

			/// Returns a reference to the current entity.
			value_type& operator*() const {
				return *(this->operator->());
			}

		private:
			enum class IterationStage { Copied, Original, Added };

		private:
			const SetType& m_entities;
			DeltaEntities<SetType> m_deltas;
			size_t m_position;
			size_t m_size;
			IterationStage m_stage;
			typename SetType::const_iterator m_iter;
		};

		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return iterator(m_originalEntities, deltas(), 0);
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return iterator(m_originalEntities, deltas(), size());
		}

	public:
		/// Gets const references to the pending modifications.
		DeltaEntities<SetType> deltas() const {
			return DeltaEntities<SetType>(m_addedEntities, m_removedEntities, m_copiedEntities);
		}

		/// Resets all pending modifications.
		void reset() {
			m_addedEntities.clear();
			m_removedEntities.clear();
			m_copiedEntities.clear();
		}

	private:
		const SetType& m_originalEntities;
		SetType m_addedEntities;
		SetType m_removedEntities;
		SetType m_copiedEntities;
	};
}}
