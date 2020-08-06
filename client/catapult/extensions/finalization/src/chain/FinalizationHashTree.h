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
#include "catapult/model/HeightHashPair.h"
#include <unordered_set>

namespace catapult { namespace chain {

	/// Finalization hash tree.
	/// \note Any node in the tree is considered to be an ancestor and descendant of itself.
	class FinalizationHashTree {
	public:
		/// Gets the number of tree nodes.
		size_t size() const;

		/// Returns \c true if \a key is contained in this tree.
		bool contains(const model::HeightHashPair& key) const;

		/// Returns \c true if \a childKey descends from \a parentKey, inclusive.
		bool isDescendant(const model::HeightHashPair& parentKey, const model::HeightHashPair& childKey) const;

		/// Finds all ancestors of \a key, inclusive.
		std::vector<model::HeightHashPair> findAncestors(const model::HeightHashPair& key) const;

	public:
		/// Adds a branch of \a count hashes (\a pHashes) starting at \a height.
		void addBranch(Height height, const Hash256* pHashes, size_t count);

	private:
		struct TreeNode {
		public:
			explicit TreeNode(const model::HeightHashPair& key)
					: pParent(nullptr)
					, Key(key)
			{}

		public:
			constexpr bool operator==(const TreeNode& rhs) const {
				return Key == rhs.Key;
			}

		public:
			const TreeNode* pParent;
			model::HeightHashPair Key;
		};

	private:
		const TreeNode* tryFindNode(const model::HeightHashPair& key) const;

	private:
		struct TreeNodeHasher {
			size_t operator()(const TreeNode& node) const;
		};

		std::unordered_set<TreeNode, TreeNodeHasher> m_tree;
	};
}}
