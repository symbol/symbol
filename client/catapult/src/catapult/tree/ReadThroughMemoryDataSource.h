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
#include "MemoryDataSource.h"

namespace catapult { namespace tree {

	/// Patricia tree memory data source that reads through but does not write through.
	template<typename TBackingDataSource>
	class ReadThroughMemoryDataSource {
	public:
		/// Creates a data source around \a backingDataSource with specified \a verbosity.
		explicit ReadThroughMemoryDataSource(
				const TBackingDataSource& backingDataSource,
				DataSourceVerbosity verbosity = DataSourceVerbosity::Off)
				: m_backingDataSource(backingDataSource)
				, m_memoryDataSource(verbosity)
		{}

	public:
		/// Gets the number of saved nodes in the in memory cache.
		size_t size() const {
			return m_memoryDataSource.size();
		}

	public:
		/// Gets the tree node associated with \a hash.
		TreeNode get(const Hash256& hash) const {
			auto node = m_memoryDataSource.get(hash);
			return !node.empty() ? std::move(node) : m_backingDataSource.get(hash);
		}

		/// Gets all nodes in memory and passes them to \a consumer.
		void forEach(const consumer<const TreeNode&>& consumer) const {
			m_memoryDataSource.forEach(consumer);
		}

	public:
		/// Saves a leaf tree \a node.
		void set(const LeafTreeNode& node) {
			m_memoryDataSource.set(node);
		}

		/// Saves a branch tree \a node.
		void set(const BranchTreeNode& node) {
			m_memoryDataSource.set(node);
		}

		/// Clears all cached memory nodes.
		void clear() {
			m_memoryDataSource.clear();
		}

	private:
		const TBackingDataSource& m_backingDataSource;
		MemoryDataSource m_memoryDataSource;
	};
}}
