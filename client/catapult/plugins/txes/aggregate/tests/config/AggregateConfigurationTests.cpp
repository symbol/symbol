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

#include "src/config/AggregateConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct AggregateConfigurationTraits {
			using ConfigurationType = AggregateConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "maxTransactionsPerAggregate", "674" },
							{ "maxCosignaturesPerAggregate", "52" },

							{ "enableStrictCosignatureCheck", "true" },
							{ "enableBondedAggregateSupport", "true" },

							{ "maxBondedTransactionLifetime", "14m" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const AggregateConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxTransactionsPerAggregate);
				EXPECT_EQ(0u, config.MaxCosignaturesPerAggregate);

				EXPECT_FALSE(config.EnableStrictCosignatureCheck);
				EXPECT_FALSE(config.EnableBondedAggregateSupport);

				EXPECT_EQ(utils::TimeSpan(), config.MaxBondedTransactionLifetime);
			}

			static void AssertCustom(const AggregateConfiguration& config) {
				// Assert:
				EXPECT_EQ(674u, config.MaxTransactionsPerAggregate);
				EXPECT_EQ(52u, config.MaxCosignaturesPerAggregate);

				EXPECT_TRUE(config.EnableStrictCosignatureCheck);
				EXPECT_TRUE(config.EnableBondedAggregateSupport);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(14), config.MaxBondedTransactionLifetime);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(AggregateConfigurationTests, Aggregate)
}}
