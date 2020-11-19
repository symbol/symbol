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
#include "PatriciaTree.h"
#include "ReadThroughMemoryDataSource.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace tree {

	/// Delta on top of a base patricia tree that offers methods to set/unset nodes.
	template<typename TEncoder, typename TDataSource, typename THasher>
	class BasePatriciaTreeDelta {
	private:
		using KeyType = typename TEncoder::KeyType;
		using ValueType = typename TEncoder::ValueType;

	public:
		/// Creates a tree around \a dataSource with root \a rootHash.
		BasePatriciaTreeDelta(const TDataSource& dataSource, const Hash256& rootHash)
				: m_dataSource(dataSource)
				, m_baseRootHash(rootHash)
				, m_tree(m_dataSource) {
			m_tree.tryLoad(rootHash);
		}

	public:
		/// Gets the root hash that uniquely identifies this tree.
		Hash256 root() const {
			return m_tree.root();
		}

		/// Gets the base root hash that identifies this tree before any changes are applied.
		Hash256 baseRoot() const {
			return m_baseRootHash;
		}

	public:
		/// Sets the root hash (\a rootHash).
		void reset(const Hash256& rootHash) {
			if (Hash256() == rootHash)
				m_tree.clear();
			else if (!m_tree.tryLoad(rootHash))
				CATAPULT_THROW_RUNTIME_ERROR_1("unable to load root hash", rootHash);

			m_baseRootHash = rootHash;
		}

		/// Sets the \a value associated with \a key in the tree.
		void set(const KeyType& key, const ValueType& value) {
			return m_tree.set(key, value);
		}

		/// Removes the value associated with \a key from the tree.
		bool unset(const KeyType& key) {
			return m_tree.unset(key);
		}

	public:
		/// Marks all nodes reachable at this point.
		void setCheckpoint() {
			m_tree.saveAll();
		}

	public:
		/// Copies all pending changes to \a dataSource.
		template<typename TDestinationDataSource>
		void copyPendingChangesTo(TDestinationDataSource& dataSource) const {
			m_dataSource.forEach([&dataSource](const auto& node) {
				if (node.isLeaf())
					dataSource.set(node.asLeafNode());
				else
					dataSource.set(node.asBranchNode());
			});
		}

		/// Sets the root of \a tree to the root of this delta tree.
		template<typename TTree>
		void copyRootTo(TTree& tree) const {
			auto rootHash = root();
			if (Hash256() == rootHash)
				tree.clear();
			else
				tree.setRoot(m_dataSource.get(rootHash));
		}

	private:
		ReadThroughMemoryDataSource<TDataSource> m_dataSource;
		Hash256 m_baseRootHash;
		PatriciaTree<TEncoder, ReadThroughMemoryDataSource<TDataSource>> m_tree;
	};
}}
