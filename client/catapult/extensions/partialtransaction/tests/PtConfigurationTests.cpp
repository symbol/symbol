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

#include "partialtransaction/src/PtConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace partialtransaction {

#define TEST_CLASS PtConfigurationTests

	namespace {
		struct PtConfigurationTraits {
			using ConfigurationType = PtConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"partialtransactions",
						{
							{ "cacheMaxResponseSize", "234KB" },
							{ "cacheMaxSize", "98MB" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const PtConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheMaxResponseSize);
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheMaxSize);
			}

			static void AssertCustom(const PtConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::FileSize::FromKilobytes(234), config.CacheMaxResponseSize);
				EXPECT_EQ(utils::FileSize::FromMegabytes(98), config.CacheMaxSize);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Pt)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(PtConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = PtConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ(utils::FileSize::FromMegabytes(20), config.CacheMaxResponseSize);
		EXPECT_EQ(utils::FileSize::FromMegabytes(60), config.CacheMaxSize);
	}

	// endregion
}}
