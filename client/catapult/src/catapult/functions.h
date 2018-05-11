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
#include <functional>

namespace catapult {

	/// An action function.
	using action = std::function<void ()>;

	/// A predicate function.
	template<typename... TArgs>
	using predicate = std::function<bool (TArgs...)>;

	/// A consumer function.
	template<typename... TArgs>
	using consumer = std::function<void (TArgs...)>;

	/// A (stateless) supplier function.
	template<typename T>
	using supplier = std::function<T ()>;
}
