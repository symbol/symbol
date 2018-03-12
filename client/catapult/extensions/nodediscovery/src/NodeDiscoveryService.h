#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace ionet { struct NetworkNode; } }

namespace catapult { namespace nodediscovery {

	/// Creates a registrar for a node discovery service around \a pLocalNetworkNode.
	/// \note This service is responsible for allowing nodes to discover each other.
	DECLARE_SERVICE_REGISTRAR(NodeDiscovery)(const std::shared_ptr<const ionet::NetworkNode>& pLocalNetworkNode);
}}
