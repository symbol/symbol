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

#include "finalization/src/FinalizationConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationConfigurationTests

	namespace {
		struct FinalizationConfigurationTraits {
			using ConfigurationType = FinalizationConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"finalization",
						{
							{ "enableVoting", "true" },
							{ "enableRevoteOnBoot", "true" },

							{ "size", "987" },
							{ "threshold", "579" },
							{ "stepDuration", "12s" },

							{ "shortLivedCacheMessageDuration", "53m" },
							{ "messageSynchronizationMaxResponseSize", "234KB" },

							{ "maxHashesPerPoint", "123" },
							{ "prevoteBlocksMultiple", "7" },
							{ "votingKeyDilution", "357" },

							{ "unfinalizedBlocksDuration", "7m" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const FinalizationConfiguration& config) {
				// Assert:
				EXPECT_FALSE(config.EnableVoting);
				EXPECT_FALSE(config.EnableRevoteOnBoot);

				EXPECT_EQ(0u, config.Size);
				EXPECT_EQ(0u, config.Threshold);
				EXPECT_EQ(utils::TimeSpan(), config.StepDuration);

				EXPECT_EQ(utils::TimeSpan(), config.ShortLivedCacheMessageDuration);
				EXPECT_EQ(utils::FileSize(), config.MessageSynchronizationMaxResponseSize);

				EXPECT_EQ(0u, config.MaxHashesPerPoint);
				EXPECT_EQ(0u, config.PrevoteBlocksMultiple);
				EXPECT_EQ(0u, config.VotingKeyDilution);

				EXPECT_EQ(utils::BlockSpan(), config.UnfinalizedBlocksDuration);

				EXPECT_EQ(0u, config.VotingSetGrouping);
			}

			static void AssertCustom(const FinalizationConfiguration& config) {
				// Assert:
				EXPECT_TRUE(config.EnableVoting);
				EXPECT_TRUE(config.EnableRevoteOnBoot);

				EXPECT_EQ(987u, config.Size);
				EXPECT_EQ(579u, config.Threshold);
				EXPECT_EQ(utils::TimeSpan::FromSeconds(12), config.StepDuration);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(53), config.ShortLivedCacheMessageDuration);
				EXPECT_EQ(utils::FileSize::FromKilobytes(234), config.MessageSynchronizationMaxResponseSize);

				EXPECT_EQ(123u, config.MaxHashesPerPoint);
				EXPECT_EQ(7u, config.PrevoteBlocksMultiple);
				EXPECT_EQ(357u, config.VotingKeyDilution);

				EXPECT_EQ(utils::BlockSpan::FromMinutes(7), config.UnfinalizedBlocksDuration);

				EXPECT_EQ(0u, config.VotingSetGrouping);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Finalization)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(FinalizationConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = FinalizationConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_TRUE(config.EnableVoting);
		EXPECT_FALSE(config.EnableRevoteOnBoot);

		EXPECT_EQ(10'000u, config.Size);
		EXPECT_EQ(7'000u, config.Threshold);
		EXPECT_EQ(utils::TimeSpan::FromMinutes(2), config.StepDuration);

		EXPECT_EQ(utils::TimeSpan::FromMinutes(10), config.ShortLivedCacheMessageDuration);
		EXPECT_EQ(utils::FileSize::FromMegabytes(20), config.MessageSynchronizationMaxResponseSize);

		EXPECT_EQ(256u, config.MaxHashesPerPoint);
		EXPECT_EQ(4u, config.PrevoteBlocksMultiple);
		EXPECT_EQ(32u, config.VotingKeyDilution);

		EXPECT_EQ(utils::BlockSpan(), config.UnfinalizedBlocksDuration);

		EXPECT_EQ(0u, config.VotingSetGrouping);
	}

	// endregion
}}
