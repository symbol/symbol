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

#include "TreeNode.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/IntegerMath.h"
#include "catapult/exceptions.h"

namespace catapult { namespace tree {

	namespace {
		std::vector<uint8_t> EncodeKey(const TreeNodePath& path, bool isLeaf) {
			auto numPathNibbles = path.size();
			std::vector<uint8_t> encodedKey(numPathNibbles / 2 + 1);

			auto i = 0u;
			encodedKey[0] = isLeaf ? 0x20 : 0; // set leaf flag
			if (1 == numPathNibbles % 2) {
				// set odd flag and merge in first nibble
				encodedKey[0] = static_cast<uint8_t>(encodedKey[0] | 0x10 | path.nibbleAt(0));
				++i;
			}

			for (; i < numPathNibbles; i += 2)
				encodedKey[i / 2 + 1] = static_cast<uint8_t>((path.nibbleAt(i) << 4) + path.nibbleAt(i + 1));

			return encodedKey;
		}
	}

	// region LeafTreeNode

	namespace {
		Hash256 CalculateLeafTreeNodeHash(const TreeNodePath& path, const Hash256& value) {
			crypto::Sha3_256_Builder builder;
			builder.update(EncodeKey(path, true));
			builder.update(value);

			Hash256 hash;
			builder.final(hash);
			return hash;
		}
	}

	LeafTreeNode::LeafTreeNode(const TreeNodePath& path, const Hash256& value)
			: m_path(path)
			, m_value(value)
			, m_hash(CalculateLeafTreeNodeHash(m_path, m_value))
	{}

	const TreeNodePath& LeafTreeNode::path() const {
		return m_path;
	}

	const Hash256& LeafTreeNode::value() const {
		return m_value;
	}

	const Hash256& LeafTreeNode::hash() const {
		return m_hash;
	}

	// endregion

	// region BranchTreeNode

	BranchTreeNode::BranchTreeNode(const TreeNodePath& path)
			: m_path(path)
			, m_links() // zero initialize
			, m_isDirty(true)
	{}

	const TreeNodePath& BranchTreeNode::path() const {
		return m_path;
	}

	size_t BranchTreeNode::numLinks() const {
		return m_linkSet.count();
	}

	bool BranchTreeNode::hasLink(size_t index) const {
		return m_linkSet.test(index);
	}

	const Hash256& BranchTreeNode::link(size_t index) const {
		const auto& pLinkedNode = m_linkedNodes[index];
		return pLinkedNode ? pLinkedNode->hash() : m_links[index];
	}

	std::unique_ptr<const TreeNode> BranchTreeNode::linkedNode(size_t index) const {
		const auto& pLinkedNode = m_linkedNodes[index];
		return pLinkedNode ? std::make_unique<TreeNode>(pLinkedNode->copy()) : nullptr;
	}

	uint8_t BranchTreeNode::highestLinkIndex() const {
		return static_cast<uint8_t>(utils::Log2(m_linkSet.to_ulong()));
	}

	const Hash256& BranchTreeNode::hash() const {
		if (m_isDirty) {
			crypto::Sha3_256_Builder builder;
			builder.update(EncodeKey(m_path, false));
			for (auto i = 0u; i < Max_Links; ++i)
				builder.update({ link(i).data(), sizeof(Hash256) });

			builder.final(m_hash);
			m_isDirty = false;
		}

		return m_hash;
	}

	void BranchTreeNode::setPath(const TreeNodePath& path) {
		m_path = path;
		m_isDirty = true;
	}

	void BranchTreeNode::setLink(const Hash256& link, size_t index) {
		m_links[index] = link;
		m_linkedNodes[index].reset();
		setLink(index);
	}

	void BranchTreeNode::setLink(const TreeNode& node, size_t index) {
		// m_links does not need to be explicitly cleared because m_linkedNodes takes precedence
		m_linkedNodes[index] = std::make_shared<const TreeNode>(node.copy());
		setLink(index);
	}

	void BranchTreeNode::clearLink(size_t index) {
		setLink(Hash256(), index);
		m_linkSet.reset(index);
	}

	void BranchTreeNode::compactLinks() {
		for (auto i = 0u; i < Max_Links; ++i) {
			auto& pLinkedNode = m_linkedNodes[i];
			if (!pLinkedNode)
				continue;

			m_links[i] = pLinkedNode->hash();
			pLinkedNode.reset();
		}
	}

	void BranchTreeNode::setLink(size_t index) {
		m_linkSet.set(index);
		m_isDirty = true;
	}

	// endregion

	// region TreeNode

	TreeNode::TreeNode() : m_emptyHash()
	{}

	TreeNode::TreeNode(const LeafTreeNode& node) : m_pLeafNode(std::make_unique<LeafTreeNode>(node))
	{}

	TreeNode::TreeNode(const BranchTreeNode& node) : m_pBranchNode(std::make_unique<BranchTreeNode>(node))
	{}

	bool TreeNode::empty() const {
		return !isLeaf() && !isBranch();
	}

	bool TreeNode::isBranch() const {
		return !!m_pBranchNode;
	}

	bool TreeNode::isLeaf() const {
		return !!m_pLeafNode;
	}

	const TreeNodePath& TreeNode::path() const {
		if (isLeaf())
			return m_pLeafNode->path();
		else if (isBranch())
			return m_pBranchNode->path();
		else
			return m_emptyPath;
	}

	const Hash256& TreeNode::hash() const {
		if (isLeaf())
			return m_pLeafNode->hash();
		else if (isBranch())
			return m_pBranchNode->hash();
		else
			return m_emptyHash;
	}

	void TreeNode::setPath(const TreeNodePath& path) {
		if (isLeaf())
			m_pLeafNode = std::make_unique<LeafTreeNode>(path, m_pLeafNode->value());
		else if (isBranch())
			m_pBranchNode->setPath(path);
		else
			CATAPULT_THROW_RUNTIME_ERROR("cannot change path of empty node");
	}

	const LeafTreeNode& TreeNode::asLeafNode() const {
		if (!isLeaf())
			CATAPULT_THROW_RUNTIME_ERROR("tree node is not a leaf node");

		return *m_pLeafNode;
	}

	const BranchTreeNode& TreeNode::asBranchNode() const {
		if (!isBranch())
			CATAPULT_THROW_RUNTIME_ERROR("tree node is not a branch node");

		return *m_pBranchNode;
	}

	TreeNode TreeNode::copy() const {
		if (isLeaf())
			return TreeNode(*m_pLeafNode);
		else if (isBranch())
			return TreeNode(*m_pBranchNode);
		else
			return TreeNode();
	}

	// endregion
}}
