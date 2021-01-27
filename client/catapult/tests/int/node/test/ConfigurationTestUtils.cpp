/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "ConfigurationTestUtils.h"
#include "catapult/utils/Logging.h"
#include "tests/test/net/CertificateLocator.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

namespace pt = boost::property_tree;

namespace catapult { namespace test {

	namespace {
		std::string CopyFile(const std::filesystem::path& destinationPath, const std::string& filename) {
			auto sourceFilePath = std::filesystem::path("..") / "resources" / filename;
			auto destinationFilePath = destinationPath / filename;
			std::filesystem::copy_file(sourceFilePath, destinationFilePath);
			return destinationFilePath.generic_string();
		}

		void DisableVoting(const std::string& configFilePath) {
			pt::ptree properties;
			pt::read_ini(configFilePath, properties);
			properties.put("finalization.enableVoting", false);
			pt::write_ini(configFilePath, properties);
		}

		void ClearAutoHarvesting(const std::string& configFilePath) {
			pt::ptree properties;
			pt::read_ini(configFilePath, properties);
			properties.put("harvesting.enableAutoHarvesting", false);
			pt::write_ini(configFilePath, properties);
		}
	}

	void PrepareConfiguration(const std::string& destination, NodeFlag nodeFlag) {
		auto destinationResourcesPath = std::filesystem::path(destination) / "resources";
		std::filesystem::create_directories(destinationResourcesPath);

		auto finalizationConfigFilePath = CopyFile(destinationResourcesPath, "config-finalization.properties");
		DisableVoting(finalizationConfigFilePath);

		CopyFile(destinationResourcesPath, "config-task.properties");

		GenerateCertificateDirectory((std::filesystem::path(destination) / "cert").generic_string());

		// don't copy the harvesting configuration if an api node is being simulated
		if (HasFlag(NodeFlag::Simulated_Api, nodeFlag))
			return;

		auto harvestingConfigFilePath = CopyFile(destinationResourcesPath, "config-harvesting.properties");
		if (!HasFlag(NodeFlag::Auto_Harvest, nodeFlag))
			ClearAutoHarvesting(harvestingConfigFilePath);
	}
}}
