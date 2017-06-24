#pragma once
#include "BaseSetDefaultTraits.h"
#include "DeltaEntities.h"
#include "catapult/exceptions.h"
#include <memory>

namespace catapult { namespace deltaset {

	namespace detail {

		/// Updates \a entity in \a entities.
		template<typename TSetTraits>
		void UpdateEntity(
				typename TSetTraits::SetType& entities,
				const typename TSetTraits::SetType::value_type& entity) {
			auto iter = entities.find(TSetTraits::ToKey(entity));
			if (entities.cend() != iter) {
				iter = entities.erase(iter);
				entities.insert(iter, entity);
				return;
			}

			CATAPULT_THROW_INVALID_ARGUMENT("entity not found, cannot update");
		}

		/// Applies all changes in \a deltas to \a entities.
		template<typename TSetTraits>
		void UpdateBaseSet(
				typename TSetTraits::SetType& entities,
				const DeltaEntities<typename TSetTraits::SetType>& deltas) {
			if (!deltas.Added.empty())
				entities.insert(deltas.Added.cbegin(), deltas.Added.cend());

			for (auto entity : deltas.Copied)
				UpdateEntity<TSetTraits>(entities, entity);

			for (auto entity : deltas.Removed)
				entities.erase(TSetTraits::ToKey(entity));
		}

		/// Policy for committing changes to a base set.
		template<typename TSetTraits>
		struct BaseSetCommitPolicy {
		private:
			using SetType = typename TSetTraits::SetType;

		public:
			static void Update(SetType& entities, const DeltaEntities<SetType>& deltas) {
				UpdateBaseSet<TSetTraits>(entities, deltas);
			}
		};
	}

	template<typename TEntityTraits, typename TSetTraits>
	class BaseSetDelta;

	/// A base set.
	/// \tparam TEntityTraits Traits describing the type of entity.
	/// \tparam TSetTraits Traits describing the underlying set.
	/// \tparam TCommitPolicy The policy for committing changes to a base set.
	///
	/// \note: 1) this class is not thread safe.
	///        2) if TSetTraits::SetType is an unordered set, the entity must implement operator ==
	///        3) if MutableTypeTraits are used, the entity must implement a (deep) copy
	template<
			typename TEntityTraits,
			typename TSetTraits,
			typename TCommitPolicy = detail::BaseSetCommitPolicy<TSetTraits>
	>
	class BaseSet : public utils::MoveOnly {
	public:
		using EntityType = typename TEntityTraits::EntityType;
		using SetType = typename TSetTraits::SetType;
		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = detail::FindTraits<EntityType, TSetTraits::AllowsNativeValueModification>;
		using DeltaType = BaseSetDelta<TEntityTraits, TSetTraits>;

	public:
		/// Gets a value indicating whether or not the set is empty.
		bool empty() const {
			return m_entities.empty();
		}

		/// Gets the size of this set.
		size_t size() const {
			return m_entities.size();
		}

		/// Searches for \a key in this set.
		/// Returns a pointer to the matching entity if it is found or \c nullptr if it is not found.
		typename FindTraits::ConstResultType find(const KeyType& key) const {
			auto iter = m_entities.find(key);
			return m_entities.cend() != iter ? FindTraits::ToResult(TSetTraits::ToValue(*iter)) : nullptr;
		}

		/// Returns an iterator that points to the entity with \a key if it is contained in this set,
		/// or cend() otherwise.
		auto findIterator(const KeyType& key) const {
			return m_entities.find(key);
		}

		/// Searches for \a key in this set.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const KeyType& key) const {
			return m_entities.cend() != m_entities.find(key);
		}

		/// Returns a delta based on the same original entities as this set.
		std::shared_ptr<DeltaType> rebase() {
			if (m_pWeakDelta.lock())
				CATAPULT_THROW_RUNTIME_ERROR("only a single attached delta is allowed at a time");

			auto pDelta = std::make_shared<DeltaType>(m_entities);
			m_pWeakDelta = pDelta;
			return pDelta;
		}

		/// Returns a delta based on the same original entities as this set
		/// but without the ability to commit any changes to the original set.
		std::shared_ptr<DeltaType> rebaseDetached() const {
			return std::make_shared<DeltaType>(m_entities);
		}

		/// Commits all changes in the rebased cache.
		template<typename... TArgs>
		void commit(TArgs&&... args) {
			auto pDelta = m_pWeakDelta.lock();
			if (!pDelta)
				CATAPULT_THROW_RUNTIME_ERROR("attempting to commit changes to a set without any outstanding attached deltas");

			auto deltas = pDelta->deltas();
			TCommitPolicy::Update(m_entities, deltas, std::forward<TArgs>(args)...);
			pDelta->reset();
		}

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_entities.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_entities.cend();
		}

	private:
		SetType m_entities;
		std::weak_ptr<DeltaType> m_pWeakDelta;
	};

	/// Creates a base set.
	template<typename TEntityTraits, typename TSetTraits>
	auto CreateBaseSet() {
		return std::make_shared<BaseSet<TEntityTraits, TSetTraits>>();
	}
}}
