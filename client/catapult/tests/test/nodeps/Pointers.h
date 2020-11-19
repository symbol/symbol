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
#include <memory>
#include <vector>

namespace catapult { namespace test {

	/// Creates a vector of \a count unique pointers.
	template<typename T>
	std::vector<std::unique_ptr<T>> CreatePointers(size_t count) {
		std::vector<std::unique_ptr<T>> items;
		for (auto i = 0u; i < count; ++i)
			items.push_back(std::make_unique<T>());

		return items;
	}

	/// Creates a vector of raw pointers corresponding to the unique pointers in \a items.
	template<typename T>
	std::vector<T*> GetRawPointers(const std::vector<std::unique_ptr<T>>& items) {
		std::vector<T*> rawItems;
		for (const auto& pItem : items)
			rawItems.push_back(pItem.get());

		return rawItems;
	}
}}
