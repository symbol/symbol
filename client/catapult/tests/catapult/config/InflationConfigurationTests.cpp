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

#include "catapult/config/InflationConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS InflationConfigurationTests

	// region basic tests

	namespace {
		struct InflationConfigurationTraits {
			using ConfigurationType = InflationConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"inflation",
						{
							{ "starting-at-height-12", "567" },
							{ "starting-at-height-35", "678" },
							{ "starting-at-height-159", "789" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return true;
			}

			static void AssertZero(const InflationConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.InflationCalculator.size());
			}

			static void AssertCustom(const InflationConfiguration& config) {
				// Assert:
				EXPECT_EQ(3u, config.InflationCalculator.size());
				EXPECT_TRUE(config.InflationCalculator.contains(Height(12), Amount(567)));
				EXPECT_TRUE(config.InflationCalculator.contains(Height(35), Amount(678)));
				EXPECT_TRUE(config.InflationCalculator.contains(Height(159), Amount(789)));
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Inflation)

	// endregion

	// region custom tests

	namespace {
		utils::ConfigurationBag::ValuesContainer CreateProperties(const std::vector<std::pair<std::string, std::string>>& keyValuePairs) {
			utils::ConfigurationBag::ValuesContainer properties;
			properties.emplace("inflation", keyValuePairs);
			return properties;
		}
	}

	TEST(TEST_CLASS, PropertyKeyMustHaveExpectedPrefix) {
		// Arrange: "with" instead of "at"
		auto properties = CreateProperties({ { "starting-with-height-12", "567" } });

		// Act + Assert:
		EXPECT_THROW(InflationConfiguration::LoadFromBag(std::move(properties)), utils::property_malformed_error);
	}

	TEST(TEST_CLASS, PropertyKeyMustHaveParsableUintAfterPrefix) {
		// Arrange:
		auto properties = CreateProperties({ { "starting-at-height-1x2", "567" } });

		// Act + Assert:
		EXPECT_THROW(InflationConfiguration::LoadFromBag(std::move(properties)), utils::property_malformed_error);
	}

	TEST(TEST_CLASS, PropertyKeyCannotBeZero) {
		// Arrange:
		auto properties = CreateProperties({ { "starting-at-height-0", "567" }, { "starting-at-height-12", "432" } });

		// Act + Assert:
		EXPECT_THROW(InflationConfiguration::LoadFromBag(std::move(properties)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, PropertyKeyMustBeUnique) {
		// Arrange:
		auto properties = CreateProperties({ { "starting-at-height-12", "567" }, { "starting-at-height-12", "432" } });

		// Act + Assert:
		EXPECT_THROW(InflationConfiguration::LoadFromBag(std::move(properties)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, PropertyKeysMustBeOrderedByHeight) {
		// Arrange:
		auto properties = CreateProperties({ { "starting-at-height-15", "567" }, { "starting-at-height-12", "432" } });

		// Act + Assert:
		EXPECT_THROW(InflationConfiguration::LoadFromBag(std::move(properties)), catapult_invalid_argument);
	}

	// endregion
}}
