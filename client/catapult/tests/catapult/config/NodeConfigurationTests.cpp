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

#include "catapult/config/NodeConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS NodeConfigurationTests

	namespace {
		struct NodeConfigurationTraits {
			using ConfigurationType = NodeConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"node",
						{
							{ "port", "1234" },
							{ "maxIncomingConnectionsPerIdentity", "7" },

							{ "enableAddressReuse", "true" },
							{ "enableSingleThreadPool", "true" },
							{ "enableCacheDatabaseStorage", "true" },
							{ "enableAutoSyncCleanup", "true" },

							{ "enableTransactionSpamThrottling", "true" },
							{ "transactionSpamThrottlingMaxBoostFee", "54'123" },

							{ "maxHashesPerSyncAttempt", "74" },
							{ "maxBlocksPerSyncAttempt", "50" },
							{ "maxChainBytesPerSyncAttempt", "2MB" },

							{ "shortLivedCacheTransactionDuration", "17h" },
							{ "shortLivedCacheBlockDuration", "23m" },
							{ "shortLivedCachePruneInterval", "1m" },
							{ "shortLivedCacheMaxSize", "654'123" },

							{ "minFeeMultiplier", "864" },
							{ "transactionSelectionStrategy", "maximize-fee" },
							{ "unconfirmedTransactionsCacheMaxResponseSize", "234KB" },
							{ "unconfirmedTransactionsCacheMaxSize", "98'763" },

							{ "connectTimeout", "4m" },
							{ "syncTimeout", "5m" },

							{ "socketWorkingBufferSize", "128KB" },
							{ "socketWorkingBufferSensitivity", "6225" },
							{ "maxPacketDataSize", "10MB" },

							{ "blockDisruptorSize", "1000" },
							{ "blockElementTraceInterval", "34" },
							{ "transactionDisruptorSize", "9876" },
							{ "transactionElementTraceInterval", "98" },

							{ "enableDispatcherAbortWhenFull", "true" },
							{ "enableDispatcherInputAuditing", "true" },

							{ "maxCacheDatabaseWriteBatchSize", "17KB" },
							{ "maxTrackedNodes", "222" },

							{ "minPartnerNodeVersion", "3.3.3.3" },
							{ "maxPartnerNodeVersion", "4.5.6.7" },

							{ "trustedHosts", "foo,BAR" },
							{ "localNetworks", "1.2.3.4,9.8.7.6" },
							{ "listenInterface", "2.4.8.16" }
						}
					},
					{
						"localnode",
						{
							{ "host", "alice.com" },
							{ "friendlyName", "a GREAT node" },
							{ "version", "4.1.2.3" },
							{ "roles", "Api,Peer" }
						}
					},
					{
						"outgoing_connections",
						{
							{ "maxConnections", "3" },
							{ "maxConnectionAge", "5" },
							{ "maxConnectionBanAge", "7" },
							{ "numConsecutiveFailuresBeforeBanning", "9" }
						}
					},
					{
						"incoming_connections",
						{
							{ "maxConnections", "8" },
							{ "maxConnectionAge", "13" },
							{ "maxConnectionBanAge", "16" },
							{ "numConsecutiveFailuresBeforeBanning", "19" },
							{ "backlogSize", "21" }
						}
					},
					{
						"banning",
						{
							{ "defaultBanDuration", "5h" },
							{ "maxBanDuration", "58h" },
							{ "keepAliveDuration", "589h" },
							{ "maxBannedNodes", "1928" },

							{ "numReadRateMonitoringBuckets", "7" },
							{ "readRateMonitoringBucketDuration", "9m" },
							{ "maxReadRateMonitoringTotalSize", "11KB" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const NodeConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.Port);
				EXPECT_EQ(0u, config.MaxIncomingConnectionsPerIdentity);

				EXPECT_FALSE(config.EnableAddressReuse);
				EXPECT_FALSE(config.EnableSingleThreadPool);
				EXPECT_FALSE(config.EnableCacheDatabaseStorage);
				EXPECT_FALSE(config.EnableAutoSyncCleanup);

				EXPECT_FALSE(config.EnableTransactionSpamThrottling);
				EXPECT_EQ(Amount(), config.TransactionSpamThrottlingMaxBoostFee);

				EXPECT_EQ(0u, config.MaxHashesPerSyncAttempt);
				EXPECT_EQ(0u, config.MaxBlocksPerSyncAttempt);
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.MaxChainBytesPerSyncAttempt);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ShortLivedCacheTransactionDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ShortLivedCacheBlockDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ShortLivedCachePruneInterval);
				EXPECT_EQ(0u, config.ShortLivedCacheMaxSize);

				EXPECT_EQ(BlockFeeMultiplier(0), config.MinFeeMultiplier);
				EXPECT_EQ(model::TransactionSelectionStrategy::Oldest, config.TransactionSelectionStrategy);
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.UnconfirmedTransactionsCacheMaxResponseSize);
				EXPECT_EQ(0u, config.UnconfirmedTransactionsCacheMaxSize);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ConnectTimeout);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.SyncTimeout);

				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.SocketWorkingBufferSize);
				EXPECT_EQ(0u, config.SocketWorkingBufferSensitivity);
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.MaxPacketDataSize);

				EXPECT_EQ(0u, config.BlockDisruptorSize);
				EXPECT_EQ(0u, config.BlockElementTraceInterval);
				EXPECT_EQ(0u, config.TransactionDisruptorSize);
				EXPECT_EQ(0u, config.TransactionElementTraceInterval);

				EXPECT_FALSE(config.EnableDispatcherAbortWhenFull);
				EXPECT_FALSE(config.EnableDispatcherInputAuditing);

				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.MaxCacheDatabaseWriteBatchSize);
				EXPECT_EQ(0u, config.MaxTrackedNodes);

				EXPECT_EQ(ionet::NodeVersion(), config.MinPartnerNodeVersion);
				EXPECT_EQ(ionet::NodeVersion(), config.MaxPartnerNodeVersion);

				EXPECT_TRUE(config.TrustedHosts.empty());
				EXPECT_TRUE(config.LocalNetworks.empty());
				EXPECT_EQ("", config.ListenInterface);

				EXPECT_EQ("", config.Local.Host);
				EXPECT_EQ("", config.Local.FriendlyName);
				EXPECT_EQ(ionet::NodeVersion(), config.Local.Version);
				EXPECT_EQ(ionet::NodeRoles::None, config.Local.Roles);

				EXPECT_EQ(0u, config.OutgoingConnections.MaxConnections);
				EXPECT_EQ(0u, config.OutgoingConnections.MaxConnectionAge);
				EXPECT_EQ(0u, config.OutgoingConnections.MaxConnectionBanAge);
				EXPECT_EQ(0u, config.OutgoingConnections.NumConsecutiveFailuresBeforeBanning);

				EXPECT_EQ(0u, config.IncomingConnections.MaxConnections);
				EXPECT_EQ(0u, config.IncomingConnections.MaxConnectionAge);
				EXPECT_EQ(0u, config.IncomingConnections.MaxConnectionBanAge);
				EXPECT_EQ(0u, config.IncomingConnections.NumConsecutiveFailuresBeforeBanning);
				EXPECT_EQ(0u, config.IncomingConnections.BacklogSize);

				EXPECT_EQ(utils::TimeSpan(), config.Banning.DefaultBanDuration);
				EXPECT_EQ(utils::TimeSpan(), config.Banning.MaxBanDuration);
				EXPECT_EQ(utils::TimeSpan(), config.Banning.KeepAliveDuration);
				EXPECT_EQ(0u, config.Banning.MaxBannedNodes);

				EXPECT_EQ(0u, config.Banning.NumReadRateMonitoringBuckets);
				EXPECT_EQ(utils::TimeSpan(), config.Banning.ReadRateMonitoringBucketDuration);
				EXPECT_EQ(utils::FileSize(), config.Banning.MaxReadRateMonitoringTotalSize);
			}

			static void AssertCustom(const NodeConfiguration& config) {
				// Assert:
				EXPECT_EQ(1234u, config.Port);
				EXPECT_EQ(7u, config.MaxIncomingConnectionsPerIdentity);

				EXPECT_TRUE(config.EnableAddressReuse);
				EXPECT_TRUE(config.EnableSingleThreadPool);
				EXPECT_TRUE(config.EnableCacheDatabaseStorage);
				EXPECT_TRUE(config.EnableAutoSyncCleanup);

				EXPECT_TRUE(config.EnableTransactionSpamThrottling);
				EXPECT_EQ(Amount(54'123), config.TransactionSpamThrottlingMaxBoostFee);

				EXPECT_EQ(74u, config.MaxHashesPerSyncAttempt);
				EXPECT_EQ(50u, config.MaxBlocksPerSyncAttempt);
				EXPECT_EQ(utils::FileSize::FromMegabytes(2), config.MaxChainBytesPerSyncAttempt);

				EXPECT_EQ(utils::TimeSpan::FromHours(17), config.ShortLivedCacheTransactionDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(23), config.ShortLivedCacheBlockDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(1), config.ShortLivedCachePruneInterval);
				EXPECT_EQ(654'123u, config.ShortLivedCacheMaxSize);

				EXPECT_EQ(BlockFeeMultiplier(864), config.MinFeeMultiplier);
				EXPECT_EQ(model::TransactionSelectionStrategy::Maximize_Fee, config.TransactionSelectionStrategy);
				EXPECT_EQ(utils::FileSize::FromKilobytes(234), config.UnconfirmedTransactionsCacheMaxResponseSize);
				EXPECT_EQ(98'763u, config.UnconfirmedTransactionsCacheMaxSize);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(4), config.ConnectTimeout);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(5), config.SyncTimeout);

				EXPECT_EQ(utils::FileSize::FromKilobytes(128), config.SocketWorkingBufferSize);
				EXPECT_EQ(6225u, config.SocketWorkingBufferSensitivity);
				EXPECT_EQ(utils::FileSize::FromMegabytes(10), config.MaxPacketDataSize);

				EXPECT_EQ(1000u, config.BlockDisruptorSize);
				EXPECT_EQ(34u, config.BlockElementTraceInterval);
				EXPECT_EQ(9876u, config.TransactionDisruptorSize);
				EXPECT_EQ(98u, config.TransactionElementTraceInterval);

				EXPECT_TRUE(config.EnableDispatcherAbortWhenFull);
				EXPECT_TRUE(config.EnableDispatcherInputAuditing);

				EXPECT_EQ(utils::FileSize::FromKilobytes(17), config.MaxCacheDatabaseWriteBatchSize);
				EXPECT_EQ(222u, config.MaxTrackedNodes);

				EXPECT_EQ(ionet::NodeVersion(0x03030303), config.MinPartnerNodeVersion);
				EXPECT_EQ(ionet::NodeVersion(0x04050607), config.MaxPartnerNodeVersion);

				EXPECT_EQ(std::unordered_set<std::string>({ "foo", "BAR" }), config.TrustedHosts);
				EXPECT_EQ(std::unordered_set<std::string>({ "1.2.3.4", "9.8.7.6" }), config.LocalNetworks);
				EXPECT_EQ("2.4.8.16", config.ListenInterface);

				EXPECT_EQ("alice.com", config.Local.Host);
				EXPECT_EQ("a GREAT node", config.Local.FriendlyName);
				EXPECT_EQ(ionet::NodeVersion(0x04010203), config.Local.Version);
				EXPECT_EQ(static_cast<ionet::NodeRoles>(3), config.Local.Roles);

				EXPECT_EQ(3u, config.OutgoingConnections.MaxConnections);
				EXPECT_EQ(5u, config.OutgoingConnections.MaxConnectionAge);
				EXPECT_EQ(7u, config.OutgoingConnections.MaxConnectionBanAge);
				EXPECT_EQ(9u, config.OutgoingConnections.NumConsecutiveFailuresBeforeBanning);

				EXPECT_EQ(8u, config.IncomingConnections.MaxConnections);
				EXPECT_EQ(13u, config.IncomingConnections.MaxConnectionAge);
				EXPECT_EQ(16u, config.IncomingConnections.MaxConnectionBanAge);
				EXPECT_EQ(19u, config.IncomingConnections.NumConsecutiveFailuresBeforeBanning);
				EXPECT_EQ(21u, config.IncomingConnections.BacklogSize);

				EXPECT_EQ(utils::TimeSpan::FromHours(5), config.Banning.DefaultBanDuration);
				EXPECT_EQ(utils::TimeSpan::FromHours(58), config.Banning.MaxBanDuration);
				EXPECT_EQ(utils::TimeSpan::FromHours(589), config.Banning.KeepAliveDuration);
				EXPECT_EQ(1928u, config.Banning.MaxBannedNodes);

				EXPECT_EQ(7u, config.Banning.NumReadRateMonitoringBuckets);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(9), config.Banning.ReadRateMonitoringBucketDuration);
				EXPECT_EQ(utils::FileSize::FromKilobytes(11), config.Banning.MaxReadRateMonitoringTotalSize);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(NodeConfigurationTests, Node)

	// region utils

	namespace {
		auto CreateLocalNetworks() {
			return std::unordered_set<std::string>{ "11.22.33.44", "22.33.44.55", "33.44" };
		}
	}

	TEST(TEST_CLASS, IsLocalHostReturnsTrueWhenHostIsContainedInLocalNetworks) {
		// Arrange:
		auto localNetworks = CreateLocalNetworks();

		// Act + Assert:
		EXPECT_TRUE(IsLocalHost("11.22.33.44", localNetworks));
		EXPECT_TRUE(IsLocalHost("22.33.44.55", localNetworks));
		EXPECT_TRUE(IsLocalHost("33.44.55", localNetworks));
	}

	TEST(TEST_CLASS, IsLocalHostReturnsFalseWhenHostIsNotContainedInLocalNetworks) {
		// Arrange:
		auto localNetworks = CreateLocalNetworks();

		// Act + Assert:
		EXPECT_FALSE(IsLocalHost("12.22.33.44", localNetworks));
		EXPECT_FALSE(IsLocalHost("111.222.333.444.555", localNetworks));
		EXPECT_FALSE(IsLocalHost("11.22.33", localNetworks));
		EXPECT_FALSE(IsLocalHost("1.2.3.4", localNetworks));
		EXPECT_FALSE(IsLocalHost("", localNetworks));
	}

	// endregion
}}
