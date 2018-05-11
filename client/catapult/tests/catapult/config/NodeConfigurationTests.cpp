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

	namespace {
		struct NodeConfigurationTraits {
			using ConfigurationType = NodeConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"node",
						{
							{ "port", "1234" },
							{ "apiPort", "8888" },
							{ "shouldAllowAddressReuse", "true" },
							{ "shouldUseSingleThreadPool", "true" },
							{ "shouldUseCacheDatabaseStorage", "true" },

							{ "shouldEnableTransactionSpamThrottling", "true" },
							{ "transactionSpamThrottlingMaxBoostFee", "54'123" },

							{ "maxBlocksPerSyncAttempt", "50" },
							{ "maxChainBytesPerSyncAttempt", "2MB" },

							{ "shortLivedCacheTransactionDuration", "17h" },
							{ "shortLivedCacheBlockDuration", "23m" },
							{ "shortLivedCachePruneInterval", "1m" },
							{ "shortLivedCacheMaxSize", "654'123" },

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

							{ "shouldAbortWhenDispatcherIsFull", "true" },
							{ "shouldAuditDispatcherInputs", "true" },
							{ "shouldPrecomputeTransactionAddresses", "true" },

							{ "outgoingSecurityMode", "Signed" },
							{ "incomingSecurityModes", "None, Signed" }
						}
					},
					{
						"localnode",
						{
							{ "host", "alice.com" },
							{ "friendlyName", "a GREAT node" },
							{ "version", "41" },
							{ "roles", "Api,Peer" }
						}
					},
					{
						"outgoing_connections",
						{
							{ "maxConnections", "3" },
							{ "maxConnectionAge", "5" }
						}
					},
					{
						"incoming_connections",
						{
							{ "maxConnections", "8" },
							{ "maxConnectionAge", "13" },
							{ "backlogSize", "21" }
						}
					},
					{
						"extensions",
						{
							{ "Alpha", "true" },
							{ "BETA", "false" },
							{ "gamma", "true" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string& section) {
				return "extensions" == section;
			}

			static void AssertZero(const NodeConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.Port);
				EXPECT_EQ(0u, config.ApiPort);
				EXPECT_FALSE(config.ShouldAllowAddressReuse);
				EXPECT_FALSE(config.ShouldUseSingleThreadPool);
				EXPECT_FALSE(config.ShouldUseCacheDatabaseStorage);

				EXPECT_FALSE(config.ShouldEnableTransactionSpamThrottling);
				EXPECT_EQ(Amount(), config.TransactionSpamThrottlingMaxBoostFee);

				EXPECT_EQ(0u, config.MaxBlocksPerSyncAttempt);
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.MaxChainBytesPerSyncAttempt);

				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ShortLivedCacheTransactionDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ShortLivedCacheBlockDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.ShortLivedCachePruneInterval);
				EXPECT_EQ(0u, config.ShortLivedCacheMaxSize);

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

				EXPECT_FALSE(config.ShouldAbortWhenDispatcherIsFull);
				EXPECT_FALSE(config.ShouldAuditDispatcherInputs);
				EXPECT_FALSE(config.ShouldPrecomputeTransactionAddresses);

				EXPECT_EQ(static_cast<ionet::ConnectionSecurityMode>(0), config.OutgoingSecurityMode);
				EXPECT_EQ(static_cast<ionet::ConnectionSecurityMode>(0), config.IncomingSecurityModes);

				EXPECT_EQ("", config.Local.Host);
				EXPECT_EQ("", config.Local.FriendlyName);
				EXPECT_EQ(0u, config.Local.Version);
				EXPECT_EQ(ionet::NodeRoles::None, config.Local.Roles);

				EXPECT_EQ(0u, config.OutgoingConnections.MaxConnections);
				EXPECT_EQ(0u, config.OutgoingConnections.MaxConnectionAge);

				EXPECT_EQ(0u, config.IncomingConnections.MaxConnections);
				EXPECT_EQ(0u, config.IncomingConnections.MaxConnectionAge);
				EXPECT_EQ(0u, config.IncomingConnections.BacklogSize);

				EXPECT_TRUE(config.Extensions.empty());
			}

			static void AssertCustom(const NodeConfiguration& config) {
				// Assert:
				EXPECT_EQ(1234u, config.Port);
				EXPECT_EQ(8888u, config.ApiPort);
				EXPECT_TRUE(config.ShouldAllowAddressReuse);
				EXPECT_TRUE(config.ShouldUseSingleThreadPool);
				EXPECT_TRUE(config.ShouldUseCacheDatabaseStorage);

				EXPECT_TRUE(config.ShouldEnableTransactionSpamThrottling);
				EXPECT_EQ(Amount(54'123), config.TransactionSpamThrottlingMaxBoostFee);

				EXPECT_EQ(50u, config.MaxBlocksPerSyncAttempt);
				EXPECT_EQ(utils::FileSize::FromMegabytes(2), config.MaxChainBytesPerSyncAttempt);

				EXPECT_EQ(utils::TimeSpan::FromHours(17), config.ShortLivedCacheTransactionDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(23), config.ShortLivedCacheBlockDuration);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(1), config.ShortLivedCachePruneInterval);
				EXPECT_EQ(654'123u, config.ShortLivedCacheMaxSize);

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

				EXPECT_TRUE(config.ShouldAbortWhenDispatcherIsFull);
				EXPECT_TRUE(config.ShouldAuditDispatcherInputs);
				EXPECT_TRUE(config.ShouldPrecomputeTransactionAddresses);

				EXPECT_EQ(ionet::ConnectionSecurityMode::Signed, config.OutgoingSecurityMode);
				EXPECT_EQ(ionet::ConnectionSecurityMode::None | ionet::ConnectionSecurityMode::Signed, config.IncomingSecurityModes);

				EXPECT_EQ("alice.com", config.Local.Host);
				EXPECT_EQ("a GREAT node", config.Local.FriendlyName);
				EXPECT_EQ(41u, config.Local.Version);
				EXPECT_EQ(static_cast<ionet::NodeRoles>(3), config.Local.Roles);

				EXPECT_EQ(3u, config.OutgoingConnections.MaxConnections);
				EXPECT_EQ(5u, config.OutgoingConnections.MaxConnectionAge);

				EXPECT_EQ(8u, config.IncomingConnections.MaxConnections);
				EXPECT_EQ(13u, config.IncomingConnections.MaxConnectionAge);
				EXPECT_EQ(21u, config.IncomingConnections.BacklogSize);

				EXPECT_EQ(std::unordered_set<std::string>({ "Alpha", "gamma" }), config.Extensions);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(NodeConfigurationTests, Node)
}}
