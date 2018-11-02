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
#include "DataSourceVerbosity.h"
#include "TreeNode.h"
#include "catapult/utils/Hashers.h"
#include "catapult/functions.h"
#include <unordered_map>

namespace catapult { namespace tree {

	/// A patricia tree memory data source.
	class MemoryDataSource {
	public:
		/// Creates a data source with specified \a verbosity.
		explicit MemoryDataSource(DataSourceVerbosity verbosity = DataSourceVerbosity::Off);

	public:
		/// Gets the number of saved nodes.
		size_t size() const;

	public:
		/// Gets the tree node associated with \a hash.
		std::unique_ptr<const TreeNode> get(const Hash256& hash) const;

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
		template<typename TNode>
		void save(const TNode& node) {
			// explicitly call hash() before emplace to ensure cached value is used
			// (and avoid undefined behavior of parameter evaluation order)
			auto nodeHash = node.hash();
			m_nodes.emplace(nodeHash, std::make_unique<TreeNode>(node));
		}

	private:
		bool m_isVerbose;
		std::unordered_map<Hash256, std::unique_ptr<TreeNode>, utils::ArrayHasher<Hash256>> m_nodes;
	};
}}
