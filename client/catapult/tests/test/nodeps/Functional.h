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
#include <vector>

namespace catapult { namespace test {

	/// Applies \a fun to all elements in \a container in the direction specified by \a forward.
	/// \see http://stackoverflow.com/questions/33379145/equivalent-of-python-map-function-using-lambda
	template<typename Container, typename Function>
	auto Apply(bool forward, const Container& container, Function fun) {
		std::vector<std::invoke_result_t<Function, const typename Container::value_type&>> result;
		for (const auto& element : container)
			result.push_back(fun(element));

		if (!forward)
			std::reverse(result.begin(), result.end());

		return result;
	}

	/// Applies \a pred to all elements in \a container and returns all elements for which it returns \c true.
	template<typename Container, typename Predicate>
	auto Filter(const Container& container, Predicate pred) {
		Container result;
		for (const auto& element : container)
			if (pred(element))
				result.push_back(element);

		return result;
	}
}}
