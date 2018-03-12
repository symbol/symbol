#pragma once
#include "PeersConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace catapult { namespace config {

	/// Loads configuration from \a path using \a loader.
	template<
			typename TConfigurationLoader,
			typename TConfiguration = typename std::result_of<TConfigurationLoader(const std::string&)>::type
	>
	TConfiguration LoadConfiguration(const boost::filesystem::path& path, TConfigurationLoader loader) {
		if (!boost::filesystem::exists(path)) {
			auto message = "aborting load due to missing configuration file";
			CATAPULT_LOG(fatal) << message << ": " << path;
			CATAPULT_THROW_EXCEPTION(catapult_runtime_error(message));
		}

		std::cout << "loading configuration from " << path << std::endl;
		return loader(path.generic_string());
	}

	/// Loads ini configuration from \a path.
	template<typename TConfiguration>
	TConfiguration LoadIniConfiguration(const boost::filesystem::path& path) {
		return LoadConfiguration(path, [](const auto& filePath) {
			return TConfiguration::LoadFromBag(utils::ConfigurationBag::FromPath(filePath));
		});
	}

	/// Loads peers configuration from \a path for network \a networkIdentifier.
	inline
	std::vector<ionet::Node> LoadPeersConfiguration(const boost::filesystem::path& path, model::NetworkIdentifier networkIdentifier) {
		return LoadConfiguration(path, [networkIdentifier](const auto& filePath) {
			return LoadPeersFromPath(filePath, networkIdentifier);
		});
	}
}}
