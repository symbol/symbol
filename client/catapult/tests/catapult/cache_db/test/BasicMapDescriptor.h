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
#include <string>

namespace catapult { namespace test {

	/// Basic map descriptor used in cache_db tests.
	template<typename TKey, typename TValue>
	struct BasicMapDescriptor {
	public:
		using KeyType = TKey;
		using ValueType = TValue;
		using StorageType = std::pair<const KeyType, ValueType>;

	public:
		static const auto& GetKeyFromValue(const ValueType& value) {
			return value.KeyCopy;
		}

		static const auto& ToKey(const StorageType& element) {
			return element.first;
		}

		static const auto& ToValue(const StorageType& element) {
			return element.second;
		}

		static auto ToStorage(const ValueType& value) {
			return StorageType(GetKeyFromValue(value), value);
		}
	};
}}
