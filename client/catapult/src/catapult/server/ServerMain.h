#pragma once
#include <boost/filesystem/path.hpp>
#include <functional>
#include <memory>

namespace catapult {
	namespace config { class LocalNodeConfiguration; }
	namespace crypto { class KeyPair; }
}

namespace catapult { namespace server {

	/// Prototype for creating a local node.
	/// \note Return value is a shared_ptr because unique_ptr of void is not allowed.
	using CreateLocalNodeFunc = std::function<std::shared_ptr<void> (config::LocalNodeConfiguration&&, const crypto::KeyPair&)>;

	/// Extracts the resources path from the command line arguments.
	/// \a argc commmand line arguments are accessible via \a argv.
	boost::filesystem::path GetResourcesPath(int argc, const char** argv);

	/// Main entry point for a catapult server process.
	/// \a argc commmand line arguments are accessible via \a argv.
	/// The local node is created by \a createLocalNode.
	int ServerMain(int argc, const char** argv, const CreateLocalNodeFunc& createLocalNode);
}}
