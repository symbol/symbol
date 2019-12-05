/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include <map>
#include <set>
#include <unordered_map>

namespace catapult { namespace utils { namespace traits {

	// region is_map

	/// If T is a standard map type, this struct will provide the member constant value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T>
	struct is_map : std::false_type {};

	template<typename ...TArgs>
	struct is_map<std::map<TArgs...>> : std::true_type {};

	template<typename ...TArgs>
	struct is_map<const std::map<TArgs...>> : std::true_type {};

	template<typename ...TArgs>
	struct is_map<std::unordered_map<TArgs...>> : std::true_type {};

	template<typename ...TArgs>
	struct is_map<const std::unordered_map<TArgs...>> : std::true_type {};

	/// \c true if T is a standard map type, \c false otherwise.
	template<typename T>
	inline constexpr bool is_map_v = is_map<T>::value;

	// endregion

	// region is_ordered

	/// If T is a standard ordered set or map type, this struct will provide the member constant value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T>
	struct is_ordered : std::false_type {};

	template<typename ...TArgs>
	struct is_ordered<std::set<TArgs...>> : std::true_type {};

	template<typename ...TArgs>
	struct is_ordered<const std::set<TArgs...>> : std::true_type {};

	template<typename ...TArgs>
	struct is_ordered<std::map<TArgs...>> : std::true_type {};

	template<typename ...TArgs>
	struct is_ordered<const std::map<TArgs...>> : std::true_type {};

	/// \c true if T is a standard ordered set or map type, \c false otherwise.
	template<typename T>
	inline constexpr bool is_ordered_v = is_ordered<T>::value;

	// endregion
}}}
