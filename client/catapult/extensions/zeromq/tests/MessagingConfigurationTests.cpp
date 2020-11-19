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

#include "zeromq/src/MessagingConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS MessagingConfigurationTests

	namespace {
		struct MessagingConfigurationTraits {
			using ConfigurationType = MessagingConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"messaging",
						{
							{ "listenInterface", "2.4.8.16" },
							{ "subscriberPort", "9753" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const MessagingConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.ListenInterface);
				EXPECT_EQ(0u, config.SubscriberPort);
			}

			static void AssertCustom(const MessagingConfiguration& config) {
				// Assert:
				EXPECT_EQ("2.4.8.16", config.ListenInterface);
				EXPECT_EQ(9753u, config.SubscriberPort);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Messaging)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(MessagingConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = MessagingConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ("0.0.0.0", config.ListenInterface);
		EXPECT_EQ(7902u, config.SubscriberPort);
	}

	// endregion
}}
