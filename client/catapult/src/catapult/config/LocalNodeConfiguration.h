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
#include "LoggingConfiguration.h"
#include "NodeConfiguration.h"
#include "PeersConfiguration.h"
#include "UserConfiguration.h"
#include "catapult/model/BlockChainConfiguration.h"
#include <boost/filesystem/path.hpp>

namespace catapult { namespace config {

	/// Comprehensive configuration for a local node.
	class LocalNodeConfiguration {
	public:
		/// Creates a local node configuration around \a blockChainConfig, \a nodeConfig, \a loggingConfig and \a userConfig.
		LocalNodeConfiguration(
				model::BlockChainConfiguration&& blockChainConfig,
				NodeConfiguration&& nodeConfig,
				LoggingConfiguration&& loggingConfig,
				UserConfiguration&& userConfig);

	public:
		/// Block chain configuration.
		const model::BlockChainConfiguration BlockChain;

		/// Node configuration.
		const NodeConfiguration Node;

		/// Logging configuration.
		const LoggingConfiguration Logging;

		/// User configuration.
		const UserConfiguration User;

	public:
		/// Loads a local node configuration from \a resourcesPath.
		/// \note This function is expected to be called be before logging is enabled.
		static LocalNodeConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};

	/// Extracts a node representing the local node from \a config.
	ionet::Node ToLocalNode(const LocalNodeConfiguration& config);
}}
