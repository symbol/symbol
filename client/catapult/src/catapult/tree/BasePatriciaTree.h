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
#include "BasePatriciaTreeDelta.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace tree {

	/// Base patricia tree.
	template<typename TEncoder, typename TDataSource, typename THasher = std::hash<typename TEncoder::KeyType>>
	class BasePatriciaTree {
	public:
		using KeyType = typename TEncoder::KeyType;
		using ValueType = typename TEncoder::ValueType;
		using DeltaType = BasePatriciaTreeDelta<TEncoder, TDataSource, THasher>;

	public:
		/// Creates a tree around \a dataSource.
		explicit BasePatriciaTree(TDataSource& dataSource)
				: m_dataSource(dataSource)
				, m_tree(m_dataSource)
		{}

		/// Creates a tree around \a dataSource with specified root hash (\a rootHash).
		BasePatriciaTree(TDataSource& dataSource, const Hash256& rootHash) : BasePatriciaTree(dataSource) {
			if (!m_tree.tryLoad(rootHash))
				CATAPULT_THROW_RUNTIME_ERROR_1("unable to load tree with root hash", rootHash);
		}

	public:
		/// Gets the root hash that uniquely identifies this tree.
		Hash256 root() const {
			return m_tree.root();
		}

		/// Tries to find the value associated with \a key in the tree and stores proof of existence or not in \a nodePath.
		std::pair<Hash256, bool> lookup(const KeyType& key, std::vector<TreeNode>& nodePath) const {
			return m_tree.lookup(key, nodePath);
		}

	public:
		/// Gets a delta based on the same data source as this tree.
		std::shared_ptr<DeltaType> rebase() {
			if (m_pWeakDelta.lock())
				CATAPULT_THROW_RUNTIME_ERROR("only a single attached delta is allowed at a time");

			auto pDelta = std::make_shared<DeltaType>(m_dataSource, root());
			m_pWeakDelta = pDelta;
			return pDelta;
		}

		/// Gets a delta based on the same data source as this tree
		/// but without the ability to commit any changes to the original tree.
		std::shared_ptr<DeltaType> rebaseDetached() const {
			return std::make_shared<DeltaType>(m_dataSource, root());
		}

	public:
		/// Commits all changes in the rebased tree.
		void commit() {
			auto pDelta = m_pWeakDelta.lock();
			if (!pDelta)
				CATAPULT_THROW_RUNTIME_ERROR("attempting to commit changes to a tree without any outstanding attached deltas");

			// copy all pending changes directly into the data source, update the root hash and reset the delta
			pDelta->setCheckpoint(); // commit should always create a checkpoint
			pDelta->copyPendingChangesTo(m_dataSource);
			pDelta->copyRootTo(m_tree); // cannot lookup in m_dataSource directly because of delayed write data sources
			pDelta->reset(pDelta->root());
		}

	private:
		TDataSource& m_dataSource;
		PatriciaTree<TEncoder, TDataSource> m_tree;
		std::weak_ptr<DeltaType> m_pWeakDelta;
	};
}}
