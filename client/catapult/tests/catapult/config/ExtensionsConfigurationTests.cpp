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

#include "catapult/config/ExtensionsConfiguration.h"
#include "tests/TestHarness.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"

namespace catapult {
namespace config {

    namespace {
        struct ExtensionsConfigurationTraits {
            using ConfigurationType = ExtensionsConfiguration;

            static utils::ConfigurationBag::ValuesContainer CreateProperties()
            {
                return { { "extensions", { { "Alpha", "true" }, { "BETA", "false" }, { "gamma", "true" } } } };
            }

            static bool IsSectionOptional(const std::string&)
            {
                return true;
            }

            static void AssertZero(const ExtensionsConfiguration& config)
            {
                // Assert:
                EXPECT_TRUE(config.Names.empty());
            }

            static void AssertCustom(const ExtensionsConfiguration& config)
            {
                // Assert:
                EXPECT_EQ(std::vector<std::string>({ "Alpha", "gamma" }), config.Names);
            }
        };
    }

    DEFINE_CONFIGURATION_TESTS(ExtensionsConfigurationTests, Extensions)
}
}
