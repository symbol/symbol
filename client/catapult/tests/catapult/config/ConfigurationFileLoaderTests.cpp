#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace config {

	namespace {
		constexpr auto Config_Filename = "config.properties";
		constexpr auto Not_Config_Filename = "not-config.properties";

		void CreateTemporaryDirectory(const boost::filesystem::path& directoryPath) {
			boost::filesystem::create_directories(directoryPath);
			std::ofstream fout((directoryPath / Config_Filename).generic_string());
			fout
					<< "[test]" << std::endl << std::endl
					<< "alpha = 7" << std::endl
					<< "beta = foo" << std::endl
					<< "gamma = z" << std::endl;
		}

		template<typename TFunc>
		void RunTestWithTemporaryDirectory(TFunc func) {
			// Arrange: create a temporary directory
			test::TempDirectoryGuard tempDir;
			CreateTemporaryDirectory(tempDir.name());

			// Act:
			func(boost::filesystem::path(tempDir.name()));
		}
	}

	// region LoadConfiguration

	TEST(ConfigurationFileLoaderTests, LoadConfigurationFailsIfFileDoesNotExist) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act:
			std::vector<std::string> callbackFilePaths;
			EXPECT_THROW(
					LoadConfiguration(path / Not_Config_Filename, [&callbackFilePaths](const auto& filePath) {
						callbackFilePaths.push_back(filePath);
					}),
					catapult_runtime_error);

			// Assert:
			EXPECT_TRUE(callbackFilePaths.empty());
		});
	}

	TEST(ConfigurationFileLoaderTests, LoadConfigurationSucceedsIfFileExists) {
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

	TEST(ConfigurationFileLoaderTests, LoadIniConfigurationFailsIfFileDoesNotExist) {
		// Arrange:
		RunTestWithTemporaryDirectory([](const auto& path) {
			// Act:
			EXPECT_THROW(
					LoadIniConfiguration<TestConfiguration>(path / Not_Config_Filename),
					catapult_runtime_error);
		});
	}

	TEST(ConfigurationFileLoaderTests, LoadIniConfigurationSucceedsIfFileExists) {
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
}}
