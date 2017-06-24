#pragma once
#include "catapult/local/BootedLocalNode.h"
#include <memory>

namespace catapult {
	namespace config { class LocalNodeConfiguration; }
	namespace crypto { class KeyPair; }
	namespace thread { class MultiServicePool; }
}

namespace catapult { namespace local { namespace p2p {

	/// Creates and boots a local p2p node around the specified:
	/// - key pair (\a keyPair)
	/// - configuration (\a config)
	/// - multi service pool (\a pMultiServicePool)
	std::unique_ptr<BootedLocalNode> CreateLocalNode(
			const crypto::KeyPair& keyPair,
			config::LocalNodeConfiguration&& config,
			std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool);
}}}
