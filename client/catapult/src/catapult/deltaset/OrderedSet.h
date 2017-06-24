#pragma once
#include "BaseSet.h"
#include "PruningBoundary.h"
#include <set>

namespace catapult { namespace deltaset {

	namespace detail {
		template<typename T>
		using OrderedSetType = std::set<
				typename std::remove_const<typename T::EntityType>::type,
				DefaultComparator<typename T::EntityType>>;

		/// Optionally prunes \a entities using \a pruningBoundary, which indicates the upper bound of entities
		/// to remove.
		template<typename TSet>
		void PruneBaseSet(TSet& entities, const PruningBoundary<typename TSet::value_type>& pruningBoundary) {
			if (!pruningBoundary.isSet())
				return;

			auto iter = entities.lower_bound(pruningBoundary.value());
			entities.erase(entities.cbegin(), iter);
		}

		/// Policy for committing changes to an ordered set.
		template<typename TSetTraits>
		struct OrderedSetCommitPolicy {
		private:
			using SetType = typename TSetTraits::SetType;

		public:
			template<typename TPruningBoundary>
			static void Update(
					SetType& entities,
					const DeltaEntities<SetType>& deltas,
					const TPruningBoundary& pruningBoundary) {
				UpdateBaseSet<TSetTraits>(entities, deltas);
				PruneBaseSet(entities, pruningBoundary);
			}
		};
	}

	/// A base set with ordered keys.
	template<typename TEntityTraits, typename TStorageTraits = SetStorageTraits<detail::OrderedSetType<TEntityTraits>>>
	using OrderedSet = BaseSet<
			TEntityTraits,
			TStorageTraits,
			detail::OrderedSetCommitPolicy<TStorageTraits>>;

	/// A delta on top of a base set with ordered keys.
	template<typename TEntityTraits, typename TStorageTraits = SetStorageTraits<detail::OrderedSetType<TEntityTraits>>>
	using OrderedSetDelta = BaseSetDelta<TEntityTraits, TStorageTraits>;

	/// Creates an ordered set.
	template<typename TEntityTraits>
	auto CreateOrderedSet() {
		return std::make_shared<OrderedSet<TEntityTraits>>();
	}
}}
