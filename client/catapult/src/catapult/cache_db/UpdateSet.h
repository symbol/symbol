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
#include "RdbTypedColumnContainer.h"
#include "catapult/deltaset/DeltaElements.h"

namespace catapult { namespace cache {

	/// Applies all changes in \a deltas to \a elements.
	template<typename TKeyTraits, typename TDescriptor, typename TContainer, typename TMemorySet>
	void UpdateSet(RdbTypedColumnContainer<TDescriptor, TContainer>& elements, const deltaset::DeltaElements<TMemorySet>& deltas) {
		auto size = elements.size();
		if (!deltas.HasChanges())
			return;

		for (const auto& added : deltas.Added)
			elements.insert(added);

		for (const auto& element : deltas.Copied)
			elements.insert(element);

		for (const auto& element : deltas.Removed)
			elements.remove(TKeyTraits::ToKey(element));

		size += deltas.Added.size();
		size -= deltas.Removed.size();
		elements.setSize(size);
	}

	/// Optionally prunes \a elements using \a pruningBoundary, which indicates the upper bound of elements to remove.
	template<typename TDescriptor, typename TContainer, typename TPruningBoundary>
	void PruneBaseSet(RdbTypedColumnContainer<TDescriptor, TContainer>& elements, const TPruningBoundary& pruningBoundary) {
		auto size = elements.size();
		size -= elements.prune(pruningBoundary.value());
		elements.setSize(size);
	}
}}
