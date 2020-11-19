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
#include "DeltaElements.h"
#include "catapult/exceptions.h"

namespace catapult { namespace deltaset {

	/// Applies all changes in \a deltas to \a elements.
	template<typename TKeyTraits, typename TStorageSet, typename TMemorySet>
	void UpdateSet(TStorageSet& elements, const DeltaElements<TMemorySet>& deltas) {
		if (!deltas.Added.empty())
			elements.insert(deltas.Added.cbegin(), deltas.Added.cend());

		for (const auto& element : deltas.Copied) {
			auto iter = elements.find(TKeyTraits::ToKey(element));
			if (elements.cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT("element not found, cannot update");

			iter = elements.erase(iter);
			elements.insert(iter, element);
		}

		for (const auto& element : deltas.Removed)
			elements.erase(TKeyTraits::ToKey(element));
	}

	/// Default policy for committing changes to a base set.
	template<typename TSetTraits>
	struct BaseSetCommitPolicy {
		/// Applies all changes in \a deltas to \a elements.
		static void Update(typename TSetTraits::SetType& elements, const DeltaElements<typename TSetTraits::MemorySetType>& deltas) {
			UpdateSet<typename TSetTraits::KeyTraits>(elements, deltas);
		}
	};
}}
