#pragma once
#include "BaseSet.h"
#include "PruningBoundary.h"
#include <set>

namespace catapult { namespace deltaset {

	namespace detail {
		template<typename T>
		using OrderedSetType = std::set<
				typename std::remove_const<typename T::ElementType>::type,
				DefaultComparator<typename T::ElementType>>;

		/// Optionally prunes \a elements using \a pruningBoundary, which indicates the upper bound of elements
		/// to remove.
		template<typename TSet>
		void PruneBaseSet(TSet& elements, const PruningBoundary<typename TSet::value_type>& pruningBoundary) {
			if (!pruningBoundary.isSet())
				return;

			auto iter = elements.lower_bound(pruningBoundary.value());
			elements.erase(elements.cbegin(), iter);
		}

		/// Policy for committing changes to an ordered set.
		template<typename TSetTraits>
		struct OrderedSetCommitPolicy {
		private:
			using SetType = typename TSetTraits::SetType;

		public:
			template<typename TPruningBoundary>
			static void Update(SetType& elements, const DeltaElements<SetType>& deltas, const TPruningBoundary& pruningBoundary) {
				UpdateBaseSet<TSetTraits>(elements, deltas);
				PruneBaseSet(elements, pruningBoundary);
			}
		};
	}

	/// A base set with ordered keys.
	template<typename TElementTraits, typename TStorageTraits = SetStorageTraits<detail::OrderedSetType<TElementTraits>>>
	class OrderedSet : public BaseSet<TElementTraits, TStorageTraits, detail::OrderedSetCommitPolicy<TStorageTraits>> {
	private:
		using BaseType = BaseSet<TElementTraits, TStorageTraits, detail::OrderedSetCommitPolicy<TStorageTraits>>;

	public:
		/// Indicates the set is ordered (used for capability detection in templates).
		static constexpr auto Is_Ordered = true;

	public:
		using BaseType::BaseType;
	};

	/// A delta on top of a base set with ordered keys.
	template<typename TElementTraits, typename TStorageTraits = SetStorageTraits<detail::OrderedSetType<TElementTraits>>>
	using OrderedSetDelta = BaseSetDelta<TElementTraits, TStorageTraits>;
}}
