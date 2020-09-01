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

#include "FinalizationHashTree.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace chain {

	size_t FinalizationHashTree::TreeNodeHasher::operator()(const TreeNode& node) const {
		return utils::ArrayHasher<Hash256>()(node.Key.Hash);
	}

	size_t FinalizationHashTree::size() const {
		return m_tree.size();
	}

	bool FinalizationHashTree::contains(const model::HeightHashPair& key) const {
		return !!tryFindNode(key);
	}

	bool FinalizationHashTree::isDescendant(const model::HeightHashPair& parentKey, const model::HeightHashPair& childKey) const {
		const auto* pNode = tryFindNode(childKey);
		while (pNode) {
			if (pNode->Key == parentKey)
				return true;

			pNode = pNode->pParent;
		}

		return false;
	}

	std::vector<model::HeightHashPair> FinalizationHashTree::findAncestors(const model::HeightHashPair& key) const {
		std::vector<model::HeightHashPair> ancestors;
		const auto* pNode = tryFindNode(key);
		while (pNode) {
			ancestors.push_back(pNode->Key);
			pNode = pNode->pParent;
		}

		return ancestors;
	}

	void FinalizationHashTree::addBranch(Height height, const Hash256* pHashes, size_t count) {
		const TreeNode* pParent = nullptr;
		for (auto i = 0u; i < count; ++i) {
			auto node = TreeNode({ height + Height(i), pHashes[i] });
			node.pParent = pParent;

			auto insertResultPair = m_tree.insert(node);
			pParent = &*insertResultPair.first;
		}
	}

	const FinalizationHashTree::TreeNode* FinalizationHashTree::tryFindNode(const model::HeightHashPair& key) const {
		auto iter = m_tree.find(TreeNode(key));
		return m_tree.cend() == iter ? nullptr : &*iter;
	}
}}
