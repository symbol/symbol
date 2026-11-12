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

#include "harvesting/src/HarvestingConfiguration.h"
#include "catapult/model/Address.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
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

				EXPECT_EQ(utils::KeySet(), config.AllowedDelegatedHarvestingMainPublicKeys);
			}

			static void AssertCustom(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("signing-key", config.HarvesterSigningPrivateKey);
				EXPECT_EQ("vrf-key", config.HarvesterVrfPrivateKey);

				EXPECT_TRUE(config.EnableAutoHarvesting);
				EXPECT_EQ(2u, config.MaxUnlockedAccounts);
				EXPECT_EQ(DelegatePrioritizationPolicy::Importance, config.DelegatePrioritizationPolicy);
				EXPECT_EQ(model::StringToAddress(Beneficiary_Address), config.BeneficiaryAddress);

				EXPECT_EQ(utils::KeySet(), config.AllowedDelegatedHarvestingMainPublicKeys);
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

	TEST(TEST_CLASS, CanParseAllowedDelegatedHarvestingMainPublicKeys) {
		// Arrange:
		auto properties = HarvestingConfigurationTraits::CreateProperties();
		properties["allowed_delegated_harvesting_main_public_keys"] = {
			{ "57E8D07B51E2275E39F73FF32B3CE389441D8628F0D085D3B86D43A3A04EBE9C", "true" },
			{ "4C3EC8BC1588F5BB34E24EED3941A0295E550E04A6C4BB2D6A420120B25AC700", "false" },
			{ "BEAE2119EAD87FD9E4BEB19374A13D9FD0C18CE650E1B67105135AF59D088EDA", "true" },
			{ "C3A1C0F4424D650C26C3196D36C4C08C9AEA9353D7B2C5FD211A8D4E4BABEE9D", "true" }
		};

		// Act:
		auto config = HarvestingConfiguration::LoadFromBag(utils::ConfigurationBag(std::move(properties)));

		// Assert:
		const auto expectedAllowedDelegatedHarvestingMainPublicKeys = utils::KeySet{
			utils::ParseByteArray<Key>("57E8D07B51E2275E39F73FF32B3CE389441D8628F0D085D3B86D43A3A04EBE9C"),
			utils::ParseByteArray<Key>("BEAE2119EAD87FD9E4BEB19374A13D9FD0C18CE650E1B67105135AF59D088EDA"),
			utils::ParseByteArray<Key>("C3A1C0F4424D650C26C3196D36C4C08C9AEA9353D7B2C5FD211A8D4E4BABEE9D")
		};

		EXPECT_EQ(expectedAllowedDelegatedHarvestingMainPublicKeys, config.AllowedDelegatedHarvestingMainPublicKeys);
	}

	TEST(TEST_CLASS, CannotParseAllowedDelegatedHarvestingMainPublicKeysWithInvalidPublicKey) {
		// Arrange:
		auto properties = HarvestingConfigurationTraits::CreateProperties();
		properties["allowed_delegated_harvesting_main_public_keys"] = {
			{ "57E8D07B51E2275E39F73FF32B3CE389441D8628F0D085D3B86D43A3A04EBE9C", "true" },
			{ "4C3EC8BC1588F5BB34E24EED3941A0295E550E04A6C4BB2D6A420120B25AC700", "false" },
			{ "BEAE2119EAD87FD9E4BEB19374A13D9FD0C18XE650E1B67105135AF59D088EDA", "true" }, // hex has 'X' character
			{ "C3A1C0F4424D650C26C3196D36C4C08C9AEA9353D7B2C5FD211A8D4E4BABEE9D", "true" }
		};
		const auto bag = utils::ConfigurationBag(std::move(properties));

		// Act + Assert:
		EXPECT_THROW(HarvestingConfiguration::LoadFromBag(bag), utils::property_malformed_error);
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
		EXPECT_EQ(test::Test_Network_Private_Keys[0], config.HarvesterSigningPrivateKey);
		EXPECT_EQ(test::Test_Network_Vrf_Private_Keys[0], config.HarvesterVrfPrivateKey);

		EXPECT_TRUE(config.EnableAutoHarvesting);
		EXPECT_EQ(5u, config.MaxUnlockedAccounts);
		EXPECT_EQ(DelegatePrioritizationPolicy::Importance, config.DelegatePrioritizationPolicy);
		EXPECT_EQ(Address(), config.BeneficiaryAddress);

		EXPECT_EQ(utils::KeySet(), config.AllowedDelegatedHarvestingMainPublicKeys);
	}

	// endregion
}}
