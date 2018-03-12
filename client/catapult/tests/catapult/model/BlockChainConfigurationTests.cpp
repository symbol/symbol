#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/ConfigurationUtils.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockChainConfigurationTests

	// region loading

	namespace {
		constexpr auto Nemesis_Public_Key = "C738E237C98760FA72726BA13DC2A1E3C13FA67DE26AF09742E972EE4EE45E1C";
		constexpr auto Nemesis_Generation_Hash = "CE076EF4ABFBC65B046987429E274EC31506D173E91BF102F16BEB7FB8176230";

		struct BlockChainConfigurationTraits {
			using ConfigurationType = BlockChainConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"network",
						{
							{ "identifier", "public-test" },
							{ "publicKey", Nemesis_Public_Key },
							{ "generationHash", Nemesis_Generation_Hash }
						}
					},
					{
						"chain",
						{
							{ "blockGenerationTargetTime", "10m" },
							{ "blockTimeSmoothingFactor", "765" },

							{ "importanceGrouping", "444" },
							{ "maxRollbackBlocks", "720" },
							{ "maxDifficultyBlocks", "15" },

							{ "maxTransactionLifetime", "30m" },
							{ "maxBlockFutureTime", "21m" },

							{ "totalChainBalance", "88'000'000'000" },
							{ "minHarvesterBalance", "4'000'000'000" },

							{ "blockPruneInterval", "432" },
							{ "maxTransactionsPerBlock", "120" }
						}
					},
					{
						"plugin:alpha",
						{
							{ "foo", "alpha" }
						}
					},
					{
						"plugin:beta",
						{
							{ "bar", "11" },
							{ "baz", "zeta" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string& section) {
				return "network" != section && "chain" != section;
			}

			static void AssertZero(const BlockChainConfiguration& config) {
				// Assert:
				EXPECT_EQ(NetworkIdentifier::Zero, config.Network.Identifier);
				EXPECT_EQ(Key{}, config.Network.PublicKey);
				EXPECT_EQ(Hash256{}, config.Network.GenerationHash);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.BlockGenerationTargetTime);
				EXPECT_EQ(0u, config.BlockTimeSmoothingFactor);

				EXPECT_EQ(0u, config.ImportanceGrouping);
				EXPECT_EQ(0u, config.MaxRollbackBlocks);
				EXPECT_EQ(0u, config.MaxDifficultyBlocks);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.MaxTransactionLifetime);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.MaxBlockFutureTime);

				EXPECT_EQ(Amount(0), config.TotalChainBalance);
				EXPECT_EQ(Amount(0), config.MinHarvesterBalance);

				EXPECT_EQ(0u, config.BlockPruneInterval);
				EXPECT_EQ(0u, config.MaxTransactionsPerBlock);

				EXPECT_TRUE(config.Plugins.empty());
			}

			static void AssertCustom(const BlockChainConfiguration& config) {
				// Assert: notice that ParseKey also works for Hash256 because it is the same type as Key
				EXPECT_EQ(NetworkIdentifier::Public_Test, config.Network.Identifier);
				EXPECT_EQ(crypto::ParseKey(Nemesis_Public_Key), config.Network.PublicKey);
				EXPECT_EQ(crypto::ParseKey(Nemesis_Generation_Hash), config.Network.GenerationHash);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(10), config.BlockGenerationTargetTime);
				EXPECT_EQ(765u, config.BlockTimeSmoothingFactor);

				EXPECT_EQ(444u, config.ImportanceGrouping);
				EXPECT_EQ(720u, config.MaxRollbackBlocks);
				EXPECT_EQ(15u, config.MaxDifficultyBlocks);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(30), config.MaxTransactionLifetime);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(21), config.MaxBlockFutureTime);

				EXPECT_EQ(Amount(88'000'000'000), config.TotalChainBalance);
				EXPECT_EQ(Amount(4'000'000'000), config.MinHarvesterBalance);

				EXPECT_EQ(432u, config.BlockPruneInterval);
				EXPECT_EQ(120u, config.MaxTransactionsPerBlock);

				EXPECT_EQ(2u, config.Plugins.size());
				const auto& pluginAlphaBag = config.Plugins.find("alpha")->second;
				EXPECT_EQ(1u, pluginAlphaBag.size());
				EXPECT_EQ("alpha", pluginAlphaBag.get<std::string>({ "", "foo" }));

				const auto& pluginBetaBag = config.Plugins.find("beta")->second;
				EXPECT_EQ(2u, pluginBetaBag.size());
				EXPECT_EQ(11u, pluginBetaBag.get<uint64_t>({ "", "bar" }));
				EXPECT_EQ("zeta", pluginBetaBag.get<std::string>({ "", "baz" }));
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, BlockChain)

	TEST(TEST_CLASS, CannotLoadBlockChainConfigurationWithInvalidNetwork) {
		// Arrange: set an unknown network in the container
		using Traits = BlockChainConfigurationTraits;
		auto container = Traits::CreateProperties();
		container["network"]["identifier"] = "foonet";

		// Act + Assert:
		EXPECT_THROW(Traits::ConfigurationType::LoadFromBag(std::move(container)), utils::property_malformed_error);
	}

	namespace {
		void AssertCannotLoadWithSection(const std::string& section) {
			// Arrange:
			using Traits = BlockChainConfigurationTraits;
			CATAPULT_LOG(debug) << "attempting to load configuration with extra section '" << section << "'";

			// - create the properties and add the desired section
			auto properties = Traits::CreateProperties();
			properties.insert({ section, { { "foo", "1234" } } });

			// Act + Assert: the load failed
			EXPECT_THROW(Traits::ConfigurationType::LoadFromBag(std::move(properties)), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, ParseFailsWhenPluginSectionNameIsNotWellFormed) {
		// Arrange: section name must start with 'plugin:' and have a name
		auto invalidSectionNames = { "", "plug", "plugina", "plug:a", "plugina:a", "a plugin:", "a plugin:b", "plugin:", " plugin:a" };
		for (const auto& section : invalidSectionNames)
			AssertCannotLoadWithSection(section);
	}

	TEST(TEST_CLASS, ParseFailsWhenPluginSectionNameContainsInvalidPluginName) {
		// Arrange:
		auto invalidPluginNames = { " ", "$$$", "some long name", "Zeta", "ZETA" };
		for (const auto& pluginName : invalidPluginNames)
			AssertCannotLoadWithSection(std::string("plugin:") + pluginName);
	}

	TEST(TEST_CLASS, ParseSucceedsWhenPluginSectionNameContainsValidPluginName) {
		// Arrange:
		using Traits = BlockChainConfigurationTraits;
		auto validPluginNames = { "a", "j", "z", ".", "zeta", "some.long.name" };
		for (const auto& pluginName : validPluginNames) {
			CATAPULT_LOG(debug) << "attempting to load configuration with plugin named " << pluginName;

			// Act: create the properties and add the desired section
			auto properties = Traits::CreateProperties();
			properties.insert({ std::string("plugin:") + pluginName, { { "foo", "1234" } } });
			auto config = Traits::ConfigurationType::LoadFromBag(std::move(properties));

			// Assert:
			EXPECT_EQ(3u, config.Plugins.size());

			const auto& pluginBag = config.Plugins.find(pluginName)->second;
			EXPECT_EQ(1u, pluginBag.size());
			EXPECT_EQ(1234u, pluginBag.get<uint64_t>({ "", "foo" }));
		}
	}

	// endregion

	// region calculated properties

	namespace {
		const uint64_t One_Hour_Ms = []() { return utils::TimeSpan::FromHours(1).millis(); }();

		utils::TimeSpan TimeSpanFromMillis(uint64_t millis) {
			return utils::TimeSpan::FromMilliseconds(millis);
		}
	}

	TEST(TEST_CLASS, CanCalculateDependentSettingsFromCustomBlockChainConfiguration) {
		// Act:
		auto config = BlockChainConfiguration::Uninitialized();
		config.BlockGenerationTargetTime = TimeSpanFromMillis(30'001);
		config.MaxTransactionLifetime = TimeSpanFromMillis(One_Hour_Ms - 1);
		config.MaxRollbackBlocks = 600;
		config.MaxDifficultyBlocks = 45;

		// Assert:
		EXPECT_EQ(TimeSpanFromMillis(30'001 * 600), CalculateFullRollbackDuration(config));
		EXPECT_EQ(TimeSpanFromMillis(30'001 * 150), CalculateRollbackVariabilityBufferDuration(config));
		EXPECT_EQ(TimeSpanFromMillis(30'001 * (600 + 150)), CalculateTransactionCacheDuration(config));
		EXPECT_EQ(645u, CalculateDifficultyHistorySize(config));
	}

	TEST(TEST_CLASS, TransactionCacheDurationIncludesBufferTimeOfAtLeastOneHour) {
		// Act:
		auto config = BlockChainConfiguration::Uninitialized();
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(15);
		config.MaxTransactionLifetime = utils::TimeSpan::FromHours(2);
		config.MaxRollbackBlocks = 20;
		config.MaxDifficultyBlocks = 45;

		// Assert:
		EXPECT_EQ(TimeSpanFromMillis(15'000 * 20), CalculateFullRollbackDuration(config));
		EXPECT_EQ(utils::TimeSpan::FromHours(1), CalculateRollbackVariabilityBufferDuration(config));
		EXPECT_EQ(TimeSpanFromMillis(15'000 * 20 + One_Hour_Ms), CalculateTransactionCacheDuration(config));
		EXPECT_EQ(65u, CalculateDifficultyHistorySize(config));
	}

	// endregion

	// region LoadPluginConfiguration

	namespace {
		struct BetaConfiguration {
		public:
			uint64_t Bar;
			std::string Baz;

		public:
			static BetaConfiguration LoadFromBag(const utils::ConfigurationBag& bag) {
				BetaConfiguration config;
				utils::LoadIniProperty(bag, "", "Bar", config.Bar);
				utils::LoadIniProperty(bag, "", "Baz", config.Baz);
				utils::VerifyBagSizeLte(bag, 2);
				return config;
			}
		};
	}

	TEST(TEST_CLASS, LoadPluginConfigurationSucceedsWhenPluginConfigurationIsPresent) {
		// Arrange:
		using Traits = BlockChainConfigurationTraits;
		auto config = Traits::ConfigurationType::LoadFromBag(Traits::CreateProperties());

		// Act:
		auto betaConfig = LoadPluginConfiguration<BetaConfiguration>(config, "beta");

		// Assert:
		EXPECT_EQ(11u, betaConfig.Bar);
		EXPECT_EQ("zeta", betaConfig.Baz);
	}

	TEST(TEST_CLASS, LoadPluginConfigurationFailsWhenPluginConfigurationIsNotPresent) {
		// Arrange:
		using Traits = BlockChainConfigurationTraits;
		auto config = Traits::ConfigurationType::LoadFromBag(Traits::CreateProperties());

		// Act + Assert:
		EXPECT_THROW(LoadPluginConfiguration<BetaConfiguration>(config, "gamma"), utils::property_not_found_error);
	}

	// endregion
}}
