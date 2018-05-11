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
	/// \a createLocalNode creates the local node.
	int ServerMain(int argc, const char** argv, const CreateLocalNodeFunc& createLocalNode);
}}
