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
#include "DataSourceVerbosity.h"
#include "TreeNode.h"
#include "catapult/utils/Hashers.h"
#include "catapult/functions.h"
#include <unordered_map>

namespace catapult { namespace tree {

	/// Patricia tree memory data source.
	class MemoryDataSource {
	public:
		/// Creates a data source with specified \a verbosity.
		explicit MemoryDataSource(DataSourceVerbosity verbosity = DataSourceVerbosity::Off);

	public:
		/// Gets the number of saved nodes.
		size_t size() const;

	public:
		/// Gets the tree node associated with \a hash.
		TreeNode get(const Hash256& hash) const;

		/// Gets all nodes and passes them to \a consumer.
		void forEach(const consumer<const TreeNode&>& consumer) const;

	public:
		/// Saves a leaf tree \a node.
		void set(const LeafTreeNode& node);

		/// Saves a branch tree \a node.
		void set(const BranchTreeNode& node);

		/// Clears all nodes.
		void clear();

	private:
		bool m_isVerbose;
		std::unordered_map<Hash256, LeafTreeNode, utils::ArrayHasher<Hash256>> m_leafNodes;
		std::unordered_map<Hash256, BranchTreeNode, utils::ArrayHasher<Hash256>> m_branchNodes;
	};
}}
