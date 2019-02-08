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
#include "catapult/config/LocalNodeConfiguration.h"
#include <string>

namespace catapult { namespace test {

	/// Adds configuration for all plugins required by the default nemesis block to \a config.
	void AddNemesisPluginExtensions(model::BlockChainConfiguration& config);

	/// Adds configuration for all extensions required by api nodes to \a config.
	void AddApiPluginExtensions(config::NodeConfiguration& config);

	/// Adds configuration for all extensions required by p2p nodes to \a config.
	void AddPeerPluginExtensions(config::NodeConfiguration& config);

	/// Adds configuration for all extensions required by simple partner nodes to \a config.
	void AddSimplePartnerPluginExtensions(config::NodeConfiguration& config);

	/// Enables receipts verification on \a config.
	void EnableReceiptsVerification(config::LocalNodeConfiguration& config);

	/// Enables state verification on \a config.
	void EnableStateVerification(config::LocalNodeConfiguration& config);

	/// Creates a test configuration for a local node with a storage in the specified directory (\a dataDirectory)
	/// that includes configuration for all plugins required by the default nemesis block.
	config::LocalNodeConfiguration CreateLocalNodeConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory);
}}
