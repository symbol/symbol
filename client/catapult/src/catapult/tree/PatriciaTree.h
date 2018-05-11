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
#include "TreeNode.h"

namespace catapult { namespace tree {

	/// Represents a compact patricia tree.
	template<typename TEncoder, typename TDataSource>
	class PatriciaTree {
	private:
		using KeyType = typename TEncoder::KeyType;
		using ValueType = typename TEncoder::ValueType;

	public:
		/// Creates a tree around a \a dataSource.
		explicit PatriciaTree(TDataSource& dataSource) : m_dataSource(dataSource)
		{}

	public:
		/// Gets the root hash that uniquely identifies this tree.
		Hash256 root() const {
			return m_rootNode.hash();
		}

		// region set

	public:
		/// Sets \a key to \a value in the tree.
		void set(const KeyType& key, const ValueType& value) {
			auto keyPath = TreeNodePath(TEncoder::EncodeKey(key));
			auto encodedValue = TEncoder::EncodeValue(value);
			m_rootNode = set(m_rootNode, { keyPath, encodedValue});
			save(m_rootNode);
		}

	private:
		struct PathValuePairRef {
			const TreeNodePath& Path;
			const Hash256& Value;
		};

	private:
		TreeNode set(const TreeNode& node, const PathValuePairRef& newPair) {
			// if the node is empty, just create a new leaf
			if (node.empty())
				return TreeNode(createLeaf(newPair));

			if (node.isLeaf()) {
				const auto& leafNode = node.asLeafNode();
				// if leaf node already points to desired location, just change value
				if (leafNode.path() == newPair.Path)
					return TreeNode(createLeaf(newPair));

				// if the path is different, the leaf node needs to be split into a branch
				return TreeNode(branchLeafNode(leafNode, newPair));
			}

			// the current node is a branch node that needs updating
			return TreeNode(updateBranchLink(BranchTreeNode(node.asBranchNode()), newPair));
		}

		LeafTreeNode createLeaf(const PathValuePairRef& pair) {
			return LeafTreeNode(pair.Path, pair.Value);
		}

		// this function is called when a branch needs to replace a leaf node
		BranchTreeNode branchLeafNode(const LeafTreeNode& leafNode, const PathValuePairRef& newPair) {
			const auto& leafPath = leafNode.path();
			auto differenceIndex = FindFirstDifferenceIndex(leafPath, newPair.Path);
			auto sharedPath = leafPath.subpath(0, differenceIndex);

			// create and save the two component nodes
			auto node1 = LeafTreeNode(leafPath.subpath(differenceIndex + 1), leafNode.value());
			save(node1);

			auto node2 = LeafTreeNode(newPair.Path.subpath(differenceIndex + 1), newPair.Value);
			save(node2);

			// create the branch node with two links
			auto branchNode = BranchTreeNode(sharedPath);
			branchNode.setLink(node1.hash(), leafPath.nibbleAt(differenceIndex));
			branchNode.setLink(node2.hash(), newPair.Path.nibbleAt(differenceIndex));
			return branchNode;
		}

		// this function is called when processing a branch node
		BranchTreeNode updateBranchLink(BranchTreeNode&& branchNode, const PathValuePairRef& newPair) {
			const auto& branchPath = branchNode.path();
			auto differenceIndex = FindFirstDifferenceIndex(branchPath, newPair.Path);

			// if the path of the existing branch node is not empty and completely shared with the new node,
			// attach the new node to the end of the branch
			if (!branchPath.empty() && differenceIndex == branchPath.size()) {
				auto newPairPathContinuation = newPair.Path.subpath(differenceIndex);
				return insertNewPairIntoBranch(branchNode, TreeNodePath(), 0, { newPairPathContinuation, newPair.Value });
			}

			return insertNewPairIntoBranch(branchNode, branchPath, differenceIndex, newPair);
		}

		BranchTreeNode insertNewPairIntoBranch(
				BranchTreeNode& branchNode,
				const TreeNodePath& branchPath,
				size_t differenceIndex,
				const PathValuePairRef& newPair) {
			auto isBranchPathConsumed = branchPath.empty();

			// if the branch path is completely consumed, find the connecting node in the database
			// if it is not completely consumed, a branch is being split
			TreeNode emptyNode;
			const auto* pNextNode = isBranchPathConsumed ? m_dataSource.get(branchNode.link(newPair.Path.nibbleAt(0))) : nullptr;
			if (!pNextNode)
				pNextNode = &emptyNode;

			// attach the new node to the existing node (if there is no existing node, it will be set as a leaf)
			auto updatedNextNode = set(*pNextNode, { newPair.Path.subpath(differenceIndex + 1), newPair.Value });
			save(updatedNextNode);

			// if the new node is a link, set it directly in the existing branch node
			if (isBranchPathConsumed) {
				branchNode.setLink(updatedNextNode.hash(), newPair.Path.nibbleAt(0));
				return branchNode;
			}

			// otherwise, create a new branch node at the shared path
			auto newBranchNode = BranchTreeNode(branchPath.subpath(0, differenceIndex));
			newBranchNode.setLink(updatedNextNode.hash(), newPair.Path.nibbleAt(differenceIndex));

			// truncate the path of the original branch node so that it is connected to the new branch node
			auto branchLinkIndex = branchPath.nibbleAt(differenceIndex);
			branchNode.setPath(branchPath.subpath(differenceIndex + 1));
			save(branchNode);

			// link the new and old branch nodes
			newBranchNode.setLink(branchNode.hash(), branchLinkIndex);
			return newBranchNode;
		}

		// endregion

		// region unset

	public:
		/// Removes the value associated with \a key from the tree.
		bool unset(const KeyType& key) {
			auto keyPath = TreeNodePath(TEncoder::EncodeKey(key));
			auto canMerge = true;
			return unset(m_rootNode, keyPath, m_rootNode, canMerge);
		}

	private:
		bool unset(const TreeNode& node, const TreeNodePath& keyPath, TreeNode& updatedNode, bool& canMerge) {
			// if the node is empty, there is nothing to do
			if (node.empty())
				return false;

			auto differenceIndex = FindFirstDifferenceIndex(node.path(), keyPath);
			if (differenceIndex == keyPath.size()) {
				// matching node was found, so clear it
				updatedNode = TreeNode();
				return true;
			}

			// if the node is not a branch, no node in the tree can completely match `keyPath`
			if (!node.isBranch())
				return false;

			// look up the branch connecting with `keyPath`, if it is not found (or not found recursively), no node in the tree can match
			const auto& branchNode = node.asBranchNode();
			auto nodeLinkIndex = keyPath.nibbleAt(differenceIndex);
			const auto* pNextNode = m_dataSource.get(branchNode.link(nodeLinkIndex));

			TreeNode updatedNextNode;
			if (!pNextNode || !unset(*pNextNode, keyPath.subpath(differenceIndex + 1), updatedNextNode, canMerge))
				return false;

			// when removing a node, at most a single branch node can be collapsed
			// subsequently, just update branch links in parent branches
			updatedNode = canMerge
					? unsetBranchLink(BranchTreeNode(branchNode), nodeLinkIndex)
					: updateBranchLink(BranchTreeNode(branchNode), nodeLinkIndex, updatedNextNode.hash());

			save(updatedNode);
			canMerge = false;
			return true;
		}

		TreeNode unsetBranchLink(BranchTreeNode&& branchNode, size_t linkIndex) {
			// unset the link
			branchNode.clearLink(linkIndex);
			if (1 != branchNode.numLinks())
				return TreeNode(branchNode);

			// merge the branch if it only has a single link (if the tree state is valid, the referenced node must exist)
			auto lastLinkIndex = branchNode.highestLinkIndex();
			auto referencedNode = m_dataSource.get(branchNode.link(lastLinkIndex))->copy();

			auto mergedPath = TreeNodePath::Join(branchNode.path(), lastLinkIndex, referencedNode.path());
			referencedNode.setPath(mergedPath);
			return std::move(referencedNode);
		}

		TreeNode updateBranchLink(BranchTreeNode&& branchNode, size_t linkIndex, const Hash256& link) {
			branchNode.setLink(link, linkIndex);
			return TreeNode(branchNode);
		}

		// endregion

		// region lookup

	public:
		/// Finds the value associated with \a key in the tree or \c nullptr if no value is set.
		const Hash256* lookup(const KeyType& key) const {
			auto keyPath = TreeNodePath(TEncoder::EncodeKey(key));
			return lookup(m_rootNode, keyPath);
		}

	private:
		const Hash256* lookup(const TreeNode& node, const TreeNodePath& keyPath) const {
			// if the node is empty, there is nothing to do
			if (node.empty())
				return nullptr;

			// if the node fully matches, it must be a leaf
			auto differenceIndex = FindFirstDifferenceIndex(node.path(), keyPath);
			if (differenceIndex == keyPath.size())
				return &node.asLeafNode().value();

			// if the node is not a branch, no node in the tree can completely match `keyPath`
			if (!node.isBranch())
				return nullptr;

			// look up the branch connecting with `keyPath`, if it is not found (or not found recursively), no node in the tree can match
			const auto& branchNode = node.asBranchNode();
			auto nodeLinkIndex = keyPath.nibbleAt(differenceIndex);
			const auto* pNextNode = m_dataSource.get(branchNode.link(nodeLinkIndex));
			if (!pNextNode)
				return nullptr;

			return lookup(*pNextNode, keyPath.subpath(differenceIndex + 1));
		}

		// endregion

	private:
		template<typename TNode>
		void save(TNode& node) {
			m_dataSource.set(node);
		}

	private:
		TDataSource& m_dataSource;
		TreeNode m_rootNode;
	};
}}

