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
#include "TreeNodePath.h"
#include "catapult/types.h"
#include <bitset>
#include <memory>

namespace catapult { namespace tree {

	// region LeafTreeNode

	/// Represents a leaf tree node.
	class LeafTreeNode {
	public:
		/// Creates a leaf node with \a path and \a value.
		LeafTreeNode(const TreeNodePath& path, const Hash256& value);

	public:
		/// Gets the node path.
		const TreeNodePath& path() const;

		/// Gets the node \a value.
		const Hash256& value() const;

		/// Gets the hash representation of this node.
		const Hash256& hash() const;

	private:
		TreeNodePath m_path;
		Hash256 m_value;
		Hash256 m_hash;
	};

	// endregion

	// region BranchTreeNode

	/// Represents a branch tree node.
	class BranchTreeNode {
	public:
		/// Creates a branch node with \a path.
		explicit BranchTreeNode(const TreeNodePath& path);

	public:
		/// Gets the node path.
		const TreeNodePath& path() const;

		/// Gets the number of links set in this node.
		size_t numLinks() const;

		/// Returns \c true if this branch has a link at \a index.
		bool hasLink(size_t index) const;

		/// Gets the branch link at \a index.
		const Hash256& link(size_t index) const;

		/// Gets the index of the highest set link.
		uint8_t highestLinkIndex() const;

		/// Gets the hash representation of this node.
		const Hash256& hash() const;

	public:
		/// Sets the branch node \a path.
		void setPath(const TreeNodePath& path);

		/// Sets the branch \a link at \a index.
		void setLink(const Hash256& link, size_t index);

		/// Clears the branch link at \a index.
		void clearLink(size_t index);

	private:
		TreeNodePath m_path;
		std::array<Hash256, 16> m_links;
		std::bitset<16> m_linkSet;
		mutable Hash256 m_hash;
		mutable bool m_isDirty;
	};

	// endregion

	// region TreeNode

	/// Represents a tree node.
	class TreeNode {
	public:
		/// Creates an empty tree node.
		TreeNode();

		/// Creates a tree node from a leaf \a node.
		explicit TreeNode(const LeafTreeNode& node);

		/// Creates a tree node from a branch \a node.
		explicit TreeNode(const BranchTreeNode& node);

	public:
		/// Returns \c true if this node represents an empty node.
		bool empty() const;

		/// Returns \c true if this node represents a branch node.
		bool isBranch() const;

		/// Returns \c true if this node represents a leaf node.
		bool isLeaf() const;

	public:
		/// Gets the node path.
		const TreeNodePath& path() const;

		/// Gets the hash representation of this node.
		const Hash256& hash() const;

	public:
		/// Sets the node \a path.
		void setPath(const TreeNodePath& path);

	public:
		/// Gets a leaf node interface to this node.
		const LeafTreeNode& asLeafNode() const;

		/// Gets a branch node interface to this node.
		const BranchTreeNode& asBranchNode() const;

	public:
		/// Creates a copy of this node.
		TreeNode copy() const;

	private:
		TreeNodePath m_emptyPath;
		Hash256 m_emptyHash;
		std::unique_ptr<LeafTreeNode> m_pLeafNode;
		std::unique_ptr<BranchTreeNode> m_pBranchNode;
	};

	// endregion
}}
