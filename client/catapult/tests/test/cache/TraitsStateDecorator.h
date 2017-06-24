#pragma once
#include "catapult/utils/Traits.h"

namespace catapult { namespace test {

	/// Stateless entity creation forwarder.
	template<typename TTraits, typename TEntityTraits, typename TEnable = void>
	struct StateDecorator {
	public:
		/// Creates an entity with \a id.
		auto CreateEntity(size_t id) const {
			return TEntityTraits::CreateEntity(id);
		}

		/// Inserts entities with \a ids into \a cache.
		template<typename TCache>
		auto InsertMultiple(TCache& cache, std::initializer_list<size_t> ids) const {
			return TTraits::InsertMultiple(cache, ids);
		}

		/// Removes \a entity from \a cache.
		template<typename TCache, typename TEntity>
		auto Remove(TCache& cache, const TEntity& entity) const {
			return TTraits::Remove(cache, entity);
		}

		/// Retrieves key from \a entity.
		template<typename TEntity>
		auto ToKey(const TEntity& entity) const {
			return TEntityTraits::ToKey(entity);
		}
	};

	/// State dependent entity creation forwarder.
	template<typename TTraits, typename TEntityTraits>
	struct StateDecorator<TTraits, TEntityTraits, typename utils::traits::enable_if_type<typename TTraits::State>::type>
			: public TTraits::State {
	public:
		/// Creates an entity with \a id.
		auto CreateEntity(size_t id) {
			return TEntityTraits::CreateEntity(*this, id);
		}

		/// Inserts entities with \a ids into \a cache.
		template<typename TCache>
		auto InsertMultiple(TCache& cache, std::initializer_list<size_t> ids) {
			return TTraits::InsertMultiple(*this, cache, ids);
		}

		/// Removes entity \a entity from \a cache.
		template<typename TCache, typename TEntity>
		auto Remove(TCache& cache, const TEntity& entity) const {
			return TTraits::Remove(*this, cache, entity);
		}

		/// Retrieves key from \a entity.
		template<typename TEntity>
		auto ToKey(const TEntity& entity) const {
			return TEntityTraits::ToKey(*this, entity);
		}

	};
}}
