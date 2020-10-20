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

#include "catapult/config/LoggingConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS LoggingConfigurationTests

	namespace {
		struct LoggingConfigurationTraits {
			using ConfigurationType = LoggingConfiguration;
			using ComponentLevelsMap = utils::ConfigurationBag::UnorderedKeyValueMap<utils::LogLevel>;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"console",
						{
							{ "sinkType", "Async" },
							{ "level", "Warning" },
							{ "colorMode", "AnsiBold" }
						}
					},
					{
						"console.component.levels",
						{
							{ "net", "Trace" },
							{ "random", "Fatal" }
						}
					},
					{
						"file",
						{
							{ "sinkType", "Sync" },
							{ "level", "Fatal" },
							{ "directory", "foo" },
							{ "filePattern", "bar%4N.log" },
							{ "rotationSize", "123KB" },
							{ "maxTotalSize", "10MB" },
							{ "minFreeSpace", "987KB" }
						}
					},
					{
						"file.component.levels",
						{
							{ "io", "Info" },
							{ "net", "Warning" },
							{ "?", "Info" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string& section) {
				return "console.component.levels" == section || "file.component.levels" == section;
			}

			static void AssertZero(const BasicLoggerConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::LogSinkType::Sync, config.SinkType);
				EXPECT_EQ(utils::LogLevel::trace, config.Level);
				EXPECT_TRUE(config.ComponentLevels.empty());
			}

			static void AssertZero(const LoggingConfiguration& config) {
				// Assert:
				// - console
				AssertZero(config.Console);
				EXPECT_EQ(utils::LogColorMode::Ansi, config.Console.ColorMode);

				// - file
				AssertZero(config.File);
				EXPECT_EQ("", config.File.Directory);
				EXPECT_EQ("", config.File.FilePattern);
				EXPECT_EQ(utils::FileSize::FromBytes(0), config.File.RotationSize);
				EXPECT_EQ(utils::FileSize::FromBytes(0), config.File.MaxTotalSize);
				EXPECT_EQ(utils::FileSize::FromBytes(0), config.File.MinFreeSpace);
			}

			static void AssertCustom(const LoggingConfiguration& config) {
				// Arrange:
				ComponentLevelsMap expectedConsoleComponentLevels{
					{ "net", utils::LogLevel::trace },
					{ "random", utils::LogLevel::fatal }
				};

				ComponentLevelsMap expectedFileComponentLevels{
					{ "io", utils::LogLevel::info },
					{ "net", utils::LogLevel::warning },
					{ "?", utils::LogLevel::info }
				};

				// Assert:
				// - console (basic)
				EXPECT_EQ(utils::LogSinkType::Async, config.Console.SinkType);
				EXPECT_EQ(utils::LogLevel::warning, config.Console.Level);
				EXPECT_EQ(expectedConsoleComponentLevels, config.Console.ComponentLevels);

				// - console (specific)
				EXPECT_EQ(utils::LogColorMode::AnsiBold, config.Console.ColorMode);

				// - file (basic)
				EXPECT_EQ(utils::LogSinkType::Sync, config.File.SinkType);
				EXPECT_EQ(utils::LogLevel::fatal, config.File.Level);
				EXPECT_EQ(expectedFileComponentLevels, config.File.ComponentLevels);

				// - file (specific)
				EXPECT_EQ("foo", config.File.Directory);
				EXPECT_EQ("bar%4N.log", config.File.FilePattern);
				EXPECT_EQ(utils::FileSize::FromKilobytes(123), config.File.RotationSize);
				EXPECT_EQ(utils::FileSize::FromMegabytes(10), config.File.MaxTotalSize);
				EXPECT_EQ(utils::FileSize::FromKilobytes(987), config.File.MinFreeSpace);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Logging)

	// region logger configuration -> logger options

	namespace {
		LoggingConfiguration LoadCustomConfiguration() {
			auto bag = utils::ConfigurationBag(LoggingConfigurationTraits::CreateProperties());
			return LoggingConfiguration::LoadFromBag(bag);
		}
	}

	TEST(TEST_CLASS, CanMapToConsoleLoggerOptions) {
		// Arrange:
		auto config = LoadCustomConfiguration();

		// Act:
		auto options = GetConsoleLoggerOptions(config.Console);

		// Assert:
		EXPECT_EQ(utils::LogSinkType::Async, options.SinkType);
		EXPECT_EQ(utils::LogColorMode::AnsiBold, options.ColorMode);
	}

	TEST(TEST_CLASS, CanMapToFileLoggerOptions) {
		// Arrange:
		auto config = LoadCustomConfiguration();

		// Act:
		auto options = GetFileLoggerOptions(config.File);

		// Assert:
		EXPECT_EQ(utils::LogSinkType::Sync, options.SinkType);
		EXPECT_EQ(utils::LogColorMode::None, options.ColorMode);

		EXPECT_EQ("foo", options.Directory);
		EXPECT_EQ("bar%4N.log", options.FilePattern);
		EXPECT_EQ(123u * 1024, options.RotationSize);
		EXPECT_EQ(10u * 1024 * 1024, options.MaxTotalSize);
		EXPECT_EQ(987u * 1024, options.MinFreeSpace);
	}

	// endregion
}}
