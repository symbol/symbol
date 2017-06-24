#include "src/config/NamespaceConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		constexpr auto Namespace_Rental_Fee_Sink_Public_Key = "75D8BB873DA8F5CCA741435DE76A46AAA2840803EBBBB0E931195B048D77F88C";
		constexpr auto Mosaic_Rental_Fee_Sink_Public_Key = "F76B23F89550EF41E2FE4C6016D8829F1CB8E4ADAB1826EB4B735A25959886ED";

		struct NamespaceConfigurationTraits {
			using ConfigurationType = NamespaceConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {{
						"",
						{
							{ "maxNameSize", "123" },
							{ "maxNamespaceDuration", "234h" },
							{ "namespaceGracePeriodDuration", "20d" },
							{ "reservedRootNamespaceNames", "alpha,omega" },

							{ "namespaceRentalFeeSinkPublicKey", Namespace_Rental_Fee_Sink_Public_Key },
							{ "rootNamespaceRentalFeePerBlock", "78" },
							{ "childNamespaceRentalFee", "11223322" },

							{ "maxMosaicDuration", "2340h" },

							{ "isMosaicLevyUpdateAllowed", "true" },
							{ "maxMosaicDivisibility", "7" },
							{ "maxMosaicDivisibleUnits", "12349876" },

							{ "mosaicRentalFeeSinkPublicKey", Mosaic_Rental_Fee_Sink_Public_Key },
							{ "mosaicRentalFee", "773388" }
						}
				}};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const NamespaceConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxNameSize);
				EXPECT_EQ(utils::BlockSpan(), config.MaxNamespaceDuration);
				EXPECT_EQ(utils::BlockSpan(), config.NamespaceGracePeriodDuration);
				EXPECT_EQ(std::unordered_set<std::string>(), config.ReservedRootNamespaceNames);

				EXPECT_EQ(Key(), config.NamespaceRentalFeeSinkPublicKey);
				EXPECT_EQ(Amount(), config.RootNamespaceRentalFeePerBlock);
				EXPECT_EQ(Amount(), config.ChildNamespaceRentalFee);

				EXPECT_EQ(utils::BlockSpan(), config.MaxMosaicDuration);

				EXPECT_FALSE(config.IsMosaicLevyUpdateAllowed);
				EXPECT_EQ(0u, config.MaxMosaicDivisibility);
				EXPECT_EQ(Amount(), config.MaxMosaicDivisibleUnits);

				EXPECT_EQ(Key(), config.MosaicRentalFeeSinkPublicKey);
				EXPECT_EQ(Amount(), config.MosaicRentalFee);
			}

			static void AssertCustom(const NamespaceConfiguration& config) {
				// Assert:
				EXPECT_EQ(123u, config.MaxNameSize);
				EXPECT_EQ(utils::BlockSpan::FromHours(234), config.MaxNamespaceDuration);
				EXPECT_EQ(utils::BlockSpan::FromDays(20), config.NamespaceGracePeriodDuration);
				EXPECT_EQ((std::unordered_set<std::string>{ "alpha", "omega" }), config.ReservedRootNamespaceNames);

				EXPECT_EQ(crypto::ParseKey(Namespace_Rental_Fee_Sink_Public_Key), config.NamespaceRentalFeeSinkPublicKey);
				EXPECT_EQ(Amount(78), config.RootNamespaceRentalFeePerBlock);
				EXPECT_EQ(Amount(11223322), config.ChildNamespaceRentalFee);

				EXPECT_EQ(utils::BlockSpan::FromHours(2340), config.MaxMosaicDuration);

				EXPECT_TRUE(config.IsMosaicLevyUpdateAllowed);
				EXPECT_EQ(7u, config.MaxMosaicDivisibility);
				EXPECT_EQ(Amount(12349876), config.MaxMosaicDivisibleUnits);

				EXPECT_EQ(crypto::ParseKey(Mosaic_Rental_Fee_Sink_Public_Key), config.MosaicRentalFeeSinkPublicKey);
				EXPECT_EQ(Amount(773388), config.MosaicRentalFee);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(NamespaceConfigurationTests, Namespace)
}}
