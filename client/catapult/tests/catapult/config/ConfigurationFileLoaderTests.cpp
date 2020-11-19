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

#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>
#include <fstream>

namespace catapult { namespace config {

#define TEST_CLASS ConfigurationFileLoaderTests

	namespace {
		constexpr auto Config_Filename = "config.properties";
		constexpr auto Config_Peers_Filename = "peers.json";
		constexpr auto Not_Config_Filename = "not-config.properties";

		void CreateTemporaryDirectory(const std::filesystem::path& directoryPath) {
			std::filesystem::create_directories(directoryPath);

			std::ofstream((directoryPath / Config_Filename).generic_string().c_str(), std::ios_base::out)
					<< "[test]" << std::endl << std::endl
					<< "alpha = 7" << std::endl
					<< "beta = foo" << std::endl
					<< "gamma = z" << std::endl;

			std::ofstream((directoryPath / Config_Peers_Filename).generic_string().c_str(), std::ios_base::out)
					<< "{ \"knownPeers\": [] }" << std::endl;
		}

		template<typename TFunc>
		void RunTestWithTemporaryDirectory(TFunc func) {
			// Arrange: create a temporary directory
			test::TempDirectoryGuard tempDir;
			CreateTemporaryDirectory(tempDir.name());

			// Act:
			func(std::filesystem::path(tempDir.name()));
		}
	}

	// region LoadConfiguration

	TEST(TEST_CLASS, LoadConfigurationFailsWhenFileDoesNotExist) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act + Assert:
			std::vector<std::string> callbackFilePaths;
			EXPECT_THROW(
					LoadConfiguration(path / Not_Config_Filename, [&callbackFilePaths](const auto& filePath) {
						callbackFilePaths.push_back(filePath);
					}),
					catapult_runtime_error);
			EXPECT_TRUE(callbackFilePaths.empty());
		});
	}

	TEST(TEST_CLASS, LoadConfigurationSucceedsWhenFileExists) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act:
			std::vector<std::string> callbackFilePaths;
			LoadConfiguration(path / Config_Filename, [&callbackFilePaths](const auto& filePath) {
				callbackFilePaths.push_back(filePath);
			});

			// Assert:
			ASSERT_EQ(1u, callbackFilePaths.size());
			EXPECT_EQ((path / Config_Filename).generic_string(), callbackFilePaths[0]);
		});
	}

	// endregion

	// region LoadIniConfiguration

	namespace {
		struct TestConfiguration {
		public:
			uint32_t Alpha;
			std::string Beta;
			std::string Gamma;

		public:
			static TestConfiguration LoadFromBag(const utils::ConfigurationBag& bag) {
				TestConfiguration config;
				utils::LoadIniProperty(bag, "test", "Alpha", config.Alpha);
				utils::LoadIniProperty(bag, "test", "Beta", config.Beta);
				utils::LoadIniProperty(bag, "test", "Gamma", config.Gamma);
				return config;
			}
		};
	}

	TEST(TEST_CLASS, LoadIniConfigurationFailsWhenFileDoesNotExist) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act + Assert:
			EXPECT_THROW(LoadIniConfiguration<TestConfiguration>(path / Not_Config_Filename), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, LoadIniConfigurationSucceedsWhenFileExists) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act:
			auto config = LoadIniConfiguration<TestConfiguration>(path / Config_Filename);

			// Assert:
			EXPECT_EQ(7u, config.Alpha);
			EXPECT_EQ("foo", config.Beta);
			EXPECT_EQ("z", config.Gamma);
		});
	}

	// endregion

	// region LoadPeersConfiguration

	TEST(TEST_CLASS, LoadPeersConfigurationFailsWhenFileDoesNotExist) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act + Assert:
			EXPECT_THROW(LoadPeersConfiguration(path / Not_Config_Filename, model::UniqueNetworkFingerprint()), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, LoadPeersConfigurationSucceedsWhenFileExists) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act:
			auto nodes = LoadPeersConfiguration(path / Config_Peers_Filename, model::UniqueNetworkFingerprint());

			// Assert:
			EXPECT_TRUE(nodes.empty());
		});
	}

	// endregion
}}
