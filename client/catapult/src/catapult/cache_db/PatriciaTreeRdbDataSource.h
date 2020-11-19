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
#include "PatriciaTreeContainer.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// Patricia tree rocksdb-based data source.
	class PatriciaTreeRdbDataSource {
	public:
		/// Creates data source around \a container.
		explicit PatriciaTreeRdbDataSource(PatriciaTreeContainer& container) : m_container(container)
		{}

	public:
		/// Gets the number of saved nodes.
		size_t size() {
			return m_container.size();
		}

		/// Gets the tree node associated with \a hash.
		tree::TreeNode get(const Hash256& hash) const {
			auto iter = m_container.find(hash);
			if (m_container.cend() == iter)
				return tree::TreeNode();

			const auto& pair = *iter;
			return pair.second.copy();
		}

	public:
		/// Saves a leaf tree \a node.
		void set(const tree::LeafTreeNode& node) {
			set(tree::TreeNode(node));
		}

		/// Saves a branch tree \a node.
		void set(const tree::BranchTreeNode& node) {
			set(tree::TreeNode(node));
		}

	private:
		void set(const tree::TreeNode& node) {
			m_container.insert(std::make_pair(node.hash(), node.copy()));
		}

	private:
		PatriciaTreeContainer& m_container;
	};
}}
