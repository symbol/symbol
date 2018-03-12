#pragma once
#include "BootedLocalNode.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace local {

	/// Creates and boots a basic local node around the specified key pair (\a keyPair) and bootstrapper (\a pBootstrapper).
	std::unique_ptr<BootedLocalNode> CreateBasicLocalNode(
			const crypto::KeyPair& keyPair,
			std::unique_ptr<extensions::LocalNodeBootstrapper>&& pBootstrapper);
}}
