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

#include "catapult/config/UserConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS UserConfigurationTests

	// region UserConfiguration

	namespace {
		struct UserConfigurationTraits {
			using ConfigurationType = UserConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"account",
						{
							{ "enableDelegatedHarvestersAutoDetection", "true" }
						}
					},
					{
						"storage",
						{
							{ "dataDirectory", "./db" },
							{ "certificateDirectory", "./cert" },
							{ "pluginsDirectory", "./ext" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const UserConfiguration& config) {
				// Assert:
				EXPECT_FALSE(config.EnableDelegatedHarvestersAutoDetection);

				EXPECT_EQ("", config.DataDirectory);
				EXPECT_EQ("", config.CertificateDirectory);
				EXPECT_EQ("", config.PluginsDirectory);
			}

			static void AssertCustom(const UserConfiguration& config) {
				// Assert:
				EXPECT_TRUE(config.EnableDelegatedHarvestersAutoDetection);

				EXPECT_EQ("./db", config.DataDirectory);
				EXPECT_EQ("./cert", config.CertificateDirectory);
				EXPECT_EQ("./ext", config.PluginsDirectory);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(UserConfigurationTests, User)

	// endregion

	// region calculated properties

	TEST(TEST_CLASS, CanGetPrivateKeyPemFilename) {
		// Arrange:
		auto config = UserConfiguration::Uninitialized();
		config.CertificateDirectory = "abc";

		// Act:
		auto filename = GetPrivateKeyPemFilename(config);

		// Assert:
		EXPECT_EQ("abc/node.key.pem", filename);
	}

	// endregion
}}
