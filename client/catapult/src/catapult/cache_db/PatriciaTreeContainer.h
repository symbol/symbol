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
#include "catapult/tree/PatriciaTreeSerializer.h"
#include "catapult/tree/TreeNode.h"

namespace catapult { namespace cache {

	/// Patricia tree column descriptor.
	struct PatriciaTreeColumnDescriptor {
	public:
		using KeyType = Hash256;
		using ValueType = tree::TreeNode;
		using StorageType = std::pair<const KeyType, ValueType>;

	public:
		/// Gets the key corresponding to \a treeNode.
		static const auto& GetKeyFromValue(const ValueType& treeNode) {
			return treeNode.hash();
		}

		/// Converts a value type (\a treeNode) to a storage type.
		static auto ToStorage(const ValueType& treeNode) {
			return StorageType(GetKeyFromValue(treeNode), treeNode.copy());
		}

		/// Converts a storage type (\a element) to a key type.
		static const auto& ToKey(const StorageType& element) {
			return element.first;
		}

		/// Converts a storage type (\a element) to a value type.
		static const auto& ToValue(const StorageType& element) {
			return element.second;
		}

	public:
		using Serializer = tree::PatriciaTreeSerializer;
	};

	/// Patricia tree typed container.
	using PatriciaTreeContainer = cache::RdbTypedColumnContainer<PatriciaTreeColumnDescriptor>;
}}
