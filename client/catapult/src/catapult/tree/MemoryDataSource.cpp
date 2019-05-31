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

#include "MemoryDataSource.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace tree {

	MemoryDataSource::MemoryDataSource(DataSourceVerbosity verbosity) : m_isVerbose(DataSourceVerbosity::Verbose == verbosity)
	{}

	size_t MemoryDataSource::size() const {
		return m_nodes.size();
	}

	std::unique_ptr<const TreeNode> MemoryDataSource::get(const Hash256& hash) const {
		auto iter = m_nodes.find(hash);
		return m_nodes.cend() != iter ? std::make_unique<const TreeNode>(iter->second->copy()) : nullptr;
	}

	void MemoryDataSource::forEach(const consumer<const TreeNode&>& consumer) const {
		for (const auto& pair : m_nodes)
			consumer(*pair.second);
	}

	void MemoryDataSource::set(const LeafTreeNode& node) {
		if (m_isVerbose) {
			CATAPULT_LOG(debug)
					<< "saving leaf node: " << node.path() << ", hash = " << node.hash()
					<< ", value = " << node.value();
		}

		save(node);
	}

	void MemoryDataSource::set(const BranchTreeNode& node) {
		if (m_isVerbose) {
			CATAPULT_LOG(debug)
					<< "saving branch node: " << node.path() << ", hash = " << node.hash()
					<< ", #links " << node.numLinks();
		}

		save(node);
	}

	void MemoryDataSource::clear() {
		m_nodes.clear();
	}
}}
