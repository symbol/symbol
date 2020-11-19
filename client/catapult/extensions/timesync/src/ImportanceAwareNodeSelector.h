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
#include "catapult/extensions/NodeSelector.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/NodeInfo.h"

namespace catapult {
	namespace cache { class ImportanceView; }
	namespace ionet { class NodeContainerView; }
}

namespace catapult { namespace timesync {

	/// Node selector that selects nodes according to the importance of the account used to boot the node.
	class ImportanceAwareNodeSelector {
	public:
		using NodeSelector = std::function<ionet::NodeSet (extensions::WeightedCandidates&, uint64_t, size_t)>;

	public:
		/// Creates a selector that can pick up to \a maxNodes nodes with a minimum importance of \a minImportance
		/// that have active connections with service id \a serviceId.
		ImportanceAwareNodeSelector(ionet::ServiceIdentifier serviceId, uint8_t maxNodes, Importance minImportance);

		/// Creates a selector around a custom \a selector that can pick up to \a maxNodes nodes with
		/// a minimum importance of \a minImportance that have active connections with service id \a serviceId.
		ImportanceAwareNodeSelector(
				ionet::ServiceIdentifier serviceId,
				uint8_t maxNodes,
				Importance minImportance,
				const NodeSelector& selector);

	public:
		/// Selects nodes from \a nodeContainerView that have a minimum importance at \a height according to \a importanceView.
		ionet::NodeSet selectNodes(
				const cache::ImportanceView& importanceView,
				const ionet::NodeContainerView& nodeContainerView,
				Height height) const;

	private:
		std::pair<Importance, bool> isCandidate(
				const cache::ImportanceView& importanceView,
				const ionet::Node& node,
				const ionet::NodeInfo& nodeInfo,
				Height height) const;

	private:
		ionet::ServiceIdentifier m_serviceId;
		uint8_t m_maxNodes;
		Importance m_minImportance;
		NodeSelector m_selector;
	};
}}
