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

#include "src/config/NamespaceConfiguration.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		constexpr auto Namespace_Rental_Fee_Sink_Address = "SDUAKZJ3WPKG6T7PYH54U2Y7SCXCAGV4EPPESHQ";

		struct NamespaceConfigurationTraits {
			using ConfigurationType = NamespaceConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "maxNameSize", "123" },
							{ "maxChildNamespaces", "1234" },
							{ "maxNamespaceDepth", "99" },

							{ "minNamespaceDuration", "32h" },
							{ "maxNamespaceDuration", "234h" },
							{ "namespaceGracePeriodDuration", "20d" },
							{ "reservedRootNamespaceNames", "alpha,omega" },

							{ "namespaceRentalFeeSinkAddress", Namespace_Rental_Fee_Sink_Address },
							{ "rootNamespaceRentalFeePerBlock", "78" },
							{ "childNamespaceRentalFee", "11223322" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const NamespaceConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxNameSize);
				EXPECT_EQ(0u, config.MaxChildNamespaces);
				EXPECT_EQ(0u, config.MaxNamespaceDepth);

				EXPECT_EQ(utils::BlockSpan(), config.MinNamespaceDuration);
				EXPECT_EQ(utils::BlockSpan(), config.MaxNamespaceDuration);
				EXPECT_EQ(utils::BlockSpan(), config.NamespaceGracePeriodDuration);
				EXPECT_EQ(std::unordered_set<std::string>(), config.ReservedRootNamespaceNames);

				EXPECT_EQ(Address(), config.NamespaceRentalFeeSinkAddress);
				EXPECT_EQ(Amount(), config.RootNamespaceRentalFeePerBlock);
				EXPECT_EQ(Amount(), config.ChildNamespaceRentalFee);
			}

			static void AssertCustom(const NamespaceConfiguration& config) {
				// Assert:
				EXPECT_EQ(123u, config.MaxNameSize);
				EXPECT_EQ(1234u, config.MaxChildNamespaces);
				EXPECT_EQ(99u, config.MaxNamespaceDepth);

				EXPECT_EQ(utils::BlockSpan::FromHours(32), config.MinNamespaceDuration);
				EXPECT_EQ(utils::BlockSpan::FromHours(234), config.MaxNamespaceDuration);
				EXPECT_EQ(utils::BlockSpan::FromDays(20), config.NamespaceGracePeriodDuration);
				EXPECT_EQ((std::unordered_set<std::string>{ "alpha", "omega" }), config.ReservedRootNamespaceNames);

				EXPECT_EQ(model::StringToAddress(Namespace_Rental_Fee_Sink_Address), config.NamespaceRentalFeeSinkAddress);
				EXPECT_EQ(Amount(78), config.RootNamespaceRentalFeePerBlock);
				EXPECT_EQ(Amount(11223322), config.ChildNamespaceRentalFee);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(NamespaceConfigurationTests, Namespace)
}}
