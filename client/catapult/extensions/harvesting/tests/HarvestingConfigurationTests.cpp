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

#include "harvesting/src/HarvestingConfiguration.h"
#include "catapult/model/Address.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingConfigurationTests

	namespace {
		constexpr auto Beneficiary_Address = "SCWHKRXVBV63IDBFN4X4KQM5LBN42SYEV42TB3A";

		struct HarvestingConfigurationTraits {
			using ConfigurationType = HarvestingConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"harvesting",
						{
							{ "harvesterSigningPrivateKey", "signing-key" },
							{ "harvesterVrfPrivateKey", "vrf-key" },

							{ "enableAutoHarvesting", "true" },
							{ "maxUnlockedAccounts", "2" },
							{ "delegatePrioritizationPolicy", "Importance" },
							{ "beneficiaryAddress", Beneficiary_Address }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.HarvesterSigningPrivateKey);
				EXPECT_EQ("", config.HarvesterVrfPrivateKey);

				EXPECT_FALSE(config.EnableAutoHarvesting);
				EXPECT_EQ(0u, config.MaxUnlockedAccounts);
				EXPECT_EQ(DelegatePrioritizationPolicy::Age, config.DelegatePrioritizationPolicy);
				EXPECT_EQ(Address(), config.BeneficiaryAddress);
			}

			static void AssertCustom(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("signing-key", config.HarvesterSigningPrivateKey);
				EXPECT_EQ("vrf-key", config.HarvesterVrfPrivateKey);

				EXPECT_TRUE(config.EnableAutoHarvesting);
				EXPECT_EQ(2u, config.MaxUnlockedAccounts);
				EXPECT_EQ(DelegatePrioritizationPolicy::Importance, config.DelegatePrioritizationPolicy);
				EXPECT_EQ(model::StringToAddress(Beneficiary_Address), config.BeneficiaryAddress);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(HarvestingConfigurationTests, Harvesting)

	// region custom parsing

	TEST(TEST_CLASS, CanParseEmptyBeneficiaryAddress) {
		// Arrange: clear beneficiary address
		auto properties = HarvestingConfigurationTraits::CreateProperties();
		for (auto& pair : properties["harvesting"]) {
			if ("beneficiaryAddress" == pair.first)
				pair.second = "";
		}

		// Act:
		auto config = HarvestingConfiguration::LoadFromBag(utils::ConfigurationBag(std::move(properties)));

		// Assert:
		EXPECT_EQ(Address(), config.BeneficiaryAddress);
	}

	// endregion

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(HarvestingConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = HarvestingConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ("", config.HarvesterSigningPrivateKey);
		EXPECT_EQ("", config.HarvesterVrfPrivateKey);

		EXPECT_FALSE(config.EnableAutoHarvesting);
		EXPECT_EQ(5u, config.MaxUnlockedAccounts);
		EXPECT_EQ(DelegatePrioritizationPolicy::Importance, config.DelegatePrioritizationPolicy);
		EXPECT_EQ(Address(), config.BeneficiaryAddress);
	}

	// endregion
}}
