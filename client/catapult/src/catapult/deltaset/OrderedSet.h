/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "BaseSet.h"
#include "PruningBoundary.h"
#include <set>

namespace catapult { namespace deltaset {

	/// Optionally prunes \a elements using \a pruningBoundary, which indicates the upper bound of elements to remove.
	template<typename TSet, typename X = decltype((*reinterpret_cast<TSet*>(0)).lower_bound(typename TSet::value_type()))>
	void PruneBaseSet(TSet& elements, const PruningBoundary<typename TSet::value_type>& pruningBoundary) {
		auto iter = elements.lower_bound(pruningBoundary.value());
		elements.erase(elements.cbegin(), iter);
	}

	namespace detail {
		template<typename T>
		using OrderedSetType = std::set<std::remove_const_t<typename T::ElementType>, std::less<typename T::ElementType>>;

		/// Policy for committing changes to an ordered set.
		template<typename TSetTraits>
		struct OrderedSetCommitPolicy {
			template<typename TPruningBoundary>
			static void Update(
					typename TSetTraits::SetType& elements,
					const DeltaElements<typename TSetTraits::MemorySetType>& deltas,
					const TPruningBoundary& pruningBoundary) {
				UpdateSet<typename TSetTraits::KeyTraits>(elements, deltas);

				if (pruningBoundary.isSet())
					PruneBaseSet(elements, pruningBoundary);
			}
		};
	}

	/// Base set with ordered keys.
	template<typename TElementTraits, typename TStorageTraits = SetStorageTraits<detail::OrderedSetType<TElementTraits>>>
	class OrderedSet : public BaseSet<TElementTraits, TStorageTraits, detail::OrderedSetCommitPolicy<TStorageTraits>> {
	private:
		using BaseType = BaseSet<TElementTraits, TStorageTraits, detail::OrderedSetCommitPolicy<TStorageTraits>>;

	public:
		using BaseType::BaseType;
	};

	/// Delta on top of a base set with ordered keys.
	template<typename TElementTraits, typename TStorageTraits = SetStorageTraits<detail::OrderedSetType<TElementTraits>>>
	using OrderedSetDelta = BaseSetDelta<TElementTraits, TStorageTraits>;
}}
