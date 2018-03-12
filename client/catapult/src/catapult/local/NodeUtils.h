#pragma once

namespace catapult {
	namespace extensions { class LocalNodeBootstrapper; }
	namespace ionet { class NodeContainer; }
}

namespace catapult { namespace local {

	/// Seeds \a nodes with node information from \a bootstrapper.
	void SeedNodeContainer(ionet::NodeContainer& nodes, const extensions::LocalNodeBootstrapper& bootstrapper);
}}
