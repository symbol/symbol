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
#include "TreeNodePath.h"
#include "catapult/types.h"
#include <bitset>
#include <memory>

namespace catapult { namespace tree { class TreeNode; } }

namespace catapult { namespace tree {

	// region LeafTreeNode

	/// Represents a leaf tree node.
	class LeafTreeNode {
	public:
		/// Creates a leaf node with \a path and \a value.
		LeafTreeNode(const TreeNodePath& path, const Hash256& value);

	private:
		LeafTreeNode();

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

	private:
		friend class TreeNode;
	};

	// endregion

	// region BranchTreeNode

	/// Represents a branch tree node.
	class BranchTreeNode {
	public:
		/// Maximum number of branch links.
		static constexpr size_t Max_Links = 16;

	public:
		/// Creates a branch node with \a path.
		explicit BranchTreeNode(const TreeNodePath& path);

	private:
		BranchTreeNode();

	public:
		/// Gets the node path.
		const TreeNodePath& path() const;

		/// Gets the number of links set in this node.
		size_t numLinks() const;

		/// Returns \c true if this branch has a link at \a index.
		bool hasLink(size_t index) const;

		/// Returns \c true if this branch has a linked node at \a index.
		/// \note Linked nodes are loaded on demand. Accordingly, if a node has a link, it might not have a linked node.
		bool hasLinkedNode(size_t index) const;

		/// Gets the branch link at \a index.
		const Hash256& link(size_t index) const;

		/// Gets a copy of the linked node at \a index or \c nullptr if no linked node is present.
		TreeNode linkedNode(size_t index) const;

		/// Gets the index of the highest set link.
		uint8_t highestLinkIndex() const;

		/// Gets the hash representation of this node.
		const Hash256& hash() const;

	public:
		/// Sets the branch node \a path.
		void setPath(const TreeNodePath& path);

		/// Sets the branch \a link at \a index.
		void setLink(const Hash256& link, size_t index);

		/// Sets the branch link at \a index to \a node.
		void setLink(const TreeNode& node, size_t index);

		/// Clears the branch link at \a index.
		void clearLink(size_t index);

		/// Compacts all links by replacing node links with hash links.
		void compactLinks();

	private:
		void setLink(size_t index);

	private:
		TreeNodePath m_path;
		std::array<Hash256, BranchTreeNode::Max_Links> m_links;
		std::array<std::shared_ptr<const TreeNode>, BranchTreeNode::Max_Links> m_linkedNodes; // shared_ptr to allow copying
		std::bitset<BranchTreeNode::Max_Links> m_linkSet;
		mutable Hash256 m_hash;
		mutable bool m_isDirty;

	private:
		friend class TreeNode;
	};

	// endregion

	// region TreeNode

	/// Represents a tree node.
	class TreeNode : public utils::MoveOnly {
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
		enum class TreeNodeType { Empty, Leaf, Branch };

	private:
		LeafTreeNode m_leafNode;
		BranchTreeNode m_branchNode;
		TreeNodeType m_treeNodeType;

		// used to return references from path() and hash() when type is Empty
		TreeNodePath m_emptyPath;
		Hash256 m_emptyHash;
	};

	// endregion
}}
