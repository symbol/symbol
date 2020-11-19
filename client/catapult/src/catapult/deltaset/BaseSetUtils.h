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
#include <algorithm>

namespace catapult { namespace deltaset {

	/// Searches for any of the given \a elements in the container (\a pContainer).
	/// Returns \c true if any element is found or \c false if none is found.
	template<typename TContainer, typename TElements>
	bool ContainsAny(const TContainer& container, const TElements& elements) {
		return std::any_of(elements.cbegin(), elements.cend(), [&container](const auto& pElement) {
			return container.contains(pElement);
		});
	}

	/// Inserts all \a elements into the container (\a pContainer).
	/// \note The algorithm relies on the data used for comparing elements being immutable.
	template<typename TContainer, typename TElements>
	void InsertAll(TContainer& container, const TElements& elements) {
		for (const auto& pElement : elements)
			container.insert(pElement);
	}

	/// Removes all \a elements from the container (\a pContainer).
	template<typename TContainer, typename TElements>
	void RemoveAll(TContainer& container, const TElements& elements) {
		for (const auto& pElement : elements)
			container.remove(pElement);
	}
}}
