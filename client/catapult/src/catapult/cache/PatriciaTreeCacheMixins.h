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
#include "PatriciaTreeUtils.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	/// Mixin for adding patricia tree support to a cache.
	template<typename TTree>
	class PatriciaTreeMixin {
	public:
		/// Creates a mixin around \a pTree.
		explicit PatriciaTreeMixin(const TTree* pTree) : m_pTree(pTree)
		{}

	public:
		/// Returns \c true if merkle root is supported.
		bool supportsMerkleRoot() const {
			return !!m_pTree;
		}

		/// Tries to get the merkle root if supported.
		std::pair<Hash256, bool> tryGetMerkleRoot() const {
			return m_pTree
					? std::make_pair(m_pTree->root(), true)
					: std::make_pair(Hash256(), false);
		}

		/// Tries to find the value associated with \a key in the tree and stores proof of existence or not in \a nodePath.
		std::pair<Hash256, bool> tryLookup(const typename TTree::KeyType& key, std::vector<tree::TreeNode>& nodePath) const {
			return m_pTree
					? m_pTree->lookup(key, nodePath)
					: std::make_pair(Hash256(), false);
		}

	private:
		const TTree* m_pTree;
	};

	/// Mixin for adding patricia tree support to a delta cache.
	template<typename TSet, typename TTree>
	class PatriciaTreeDeltaMixin {
	public:
		/// Creates a mixin around delta \a set and \a pTree.
		PatriciaTreeDeltaMixin(TSet& set, const std::shared_ptr<TTree>& pTree)
				: m_set(set)
				, m_pTree(pTree)
				, m_nextGenerationId(m_set.generationId())
		{}

	public:
		/// Returns \c true if merkle root is supported.
		bool supportsMerkleRoot() const {
			return !!m_pTree;
		}

		/// Tries to get the merkle root if supported.
		std::pair<Hash256, bool> tryGetMerkleRoot() const {
			return m_pTree
					? std::make_pair(m_pTree->root(), true)
					: std::make_pair(Hash256(), false);
		}

		/// Recalculates the merkle root given the specified chain \a height if supported.
		void updateMerkleRoot(Height height) {
			if (!m_pTree)
				return;

			ApplyDeltasToTree(*m_pTree, m_set, m_nextGenerationId, height);
			setApplyCheckpoint();
		}

		/// Sets the merkle root (\a merkleRoot) if supported.
		/// \note There must not be any pending changes.
		void setMerkleRoot(const Hash256& merkleRoot) {
			if (!m_pTree)
				CATAPULT_THROW_RUNTIME_ERROR_1("cannot set merkle root", merkleRoot);

			m_pTree->reset(merkleRoot);
			setApplyCheckpoint();
		}

	private:
		void setApplyCheckpoint() {
			m_set.incrementGenerationId();
			m_nextGenerationId = m_set.generationId();
			m_pTree->setCheckpoint();
		}

	private:
		TSet& m_set;
		std::shared_ptr<TTree> m_pTree;
		uint32_t m_nextGenerationId;
	};
}}
