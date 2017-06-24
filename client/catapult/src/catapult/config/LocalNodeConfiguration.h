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
		/// Creates a local node configuration around \a blockChainConfig, \a nodeConfig, \a loggingConfig,
		/// \a userConfig and \a peers.
		LocalNodeConfiguration(
				model::BlockChainConfiguration&& blockChainConfig,
				NodeConfiguration&& nodeConfig,
				LoggingConfiguration&& loggingConfig,
				UserConfiguration&& userConfig,
				std::vector<ionet::Node>&& peers);

	public:
		/// The block chain configuration.
		const model::BlockChainConfiguration BlockChain;

		/// The node configuration.
		const NodeConfiguration Node;

		/// The logging configuration.
		const LoggingConfiguration Logging;

		/// The user configuration.
		const UserConfiguration User;

		/// The configured peers.
		const std::vector<ionet::Node> Peers;

	public:
		/// Loads a local node configuration from \a resourcesPath.
		/// \note This function is expected to be called be before logging is enabled.
		static LocalNodeConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}
