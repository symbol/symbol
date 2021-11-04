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

#include "src/config/MosaicConfiguration.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS MosaicConfigurationTests

	// region MosaicConfiguration

	namespace {
		constexpr auto Mosaic_Rental_Fee_Sink_Address_V1 = "TB4V5Q54TUMWAEXNICRP3JDDUDDFK4UVBJ7MKQA";
		constexpr auto Mosaic_Rental_Fee_Sink_Address = "TDDVCZTD4ITLQ2HKUR3EFXB22TXCOS5BZ2ZOECI";

		struct MosaicConfigurationTraits {
			using ConfigurationType = MosaicConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "maxMosaicsPerAccount", "4321" },
							{ "maxMosaicDuration", "2340h" },
							{ "maxMosaicDivisibility", "7" },

							{ "mosaicRentalFeeSinkAddressV1", Mosaic_Rental_Fee_Sink_Address_V1 },
							{ "mosaicRentalFeeSinkAddress", Mosaic_Rental_Fee_Sink_Address },
							{ "mosaicRentalFee", "773388" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const MosaicConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxMosaicsPerAccount);
				EXPECT_EQ(utils::BlockSpan(), config.MaxMosaicDuration);
				EXPECT_EQ(0u, config.MaxMosaicDivisibility);

				EXPECT_EQ(Address(), config.MosaicRentalFeeSinkAddressV1);
				EXPECT_EQ(Address(), config.MosaicRentalFeeSinkAddress);
				EXPECT_EQ(Amount(), config.MosaicRentalFee);
			}

			static void AssertCustom(const MosaicConfiguration& config) {
				// Assert:
				EXPECT_EQ(4321u, config.MaxMosaicsPerAccount);
				EXPECT_EQ(utils::BlockSpan::FromHours(2340), config.MaxMosaicDuration);
				EXPECT_EQ(7u, config.MaxMosaicDivisibility);

				EXPECT_EQ(model::StringToAddress(Mosaic_Rental_Fee_Sink_Address_V1), config.MosaicRentalFeeSinkAddressV1);
				EXPECT_EQ(model::StringToAddress(Mosaic_Rental_Fee_Sink_Address), config.MosaicRentalFeeSinkAddress);
				EXPECT_EQ(Amount(773388), config.MosaicRentalFee);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(MosaicConfigurationTests, Mosaic)

	// endregion

	// region calculated properties

	TEST(TEST_CLASS, CanGetMosaicRentalFeeSinkAddressWithoutFork) {
		// Arrange:
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		blockChainConfig.ForkHeights.TreasuryReissuance = Height();

		auto config = MosaicConfiguration::Uninitialized();
		config.MosaicRentalFeeSinkAddressV1 = test::GenerateRandomByteArray<Address>();
		config.MosaicRentalFeeSinkAddress = test::GenerateRandomByteArray<Address>();

		// Act:
		auto sinkAddress = GetMosaicRentalFeeSinkAddress(config, blockChainConfig);

		// Assert:
		EXPECT_EQ(config.MosaicRentalFeeSinkAddress, sinkAddress.get(Height(0)));
		EXPECT_EQ(config.MosaicRentalFeeSinkAddress, sinkAddress.get(Height(1)));
	}

	TEST(TEST_CLASS, CanGetMosaicRentalFeeSinkAddressWithFork) {
		// Arrange:
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		blockChainConfig.ForkHeights.TreasuryReissuance = Height(1234);

		auto config = MosaicConfiguration::Uninitialized();
		config.MosaicRentalFeeSinkAddressV1 = test::GenerateRandomByteArray<Address>();
		config.MosaicRentalFeeSinkAddress = test::GenerateRandomByteArray<Address>();

		// Act:
		auto sinkAddress = GetMosaicRentalFeeSinkAddress(config, blockChainConfig);

		// Assert:
		EXPECT_EQ(config.MosaicRentalFeeSinkAddress, sinkAddress.get(Height(0)));
		EXPECT_EQ(config.MosaicRentalFeeSinkAddressV1, sinkAddress.get(Height(1)));

		EXPECT_EQ(config.MosaicRentalFeeSinkAddressV1, sinkAddress.get(Height(1233)));
		EXPECT_EQ(config.MosaicRentalFeeSinkAddress, sinkAddress.get(Height(1234)));
		EXPECT_EQ(config.MosaicRentalFeeSinkAddress, sinkAddress.get(Height(1235)));
	}

	// endregion
}}
