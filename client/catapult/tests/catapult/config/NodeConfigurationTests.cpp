#include "catapult/config/NodeConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct NodeConfigurationTraits {
			using ConfigurationType = NodeConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {{
						"node",
						{
							{ "port", "1234" },
							{ "apiPort", "8888" },
							{ "shouldAllowAddressReuse", "true" },

							{ "minBlocksPerSyncAttempt", "25" },
							{ "maxBlocksPerSyncAttempt", "50" },
							{ "minChainBytesPerSyncAttempt", "1MB" },
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
							{ "maxPacketDataSize", "10MB" },

							{ "blockDisruptorSize", "1000" },
							{ "blockElementTraceInterval", "34" },
							{ "transactionDisruptorSize", "9876" },
							{ "transactionElementTraceInterval", "98" },
							{ "transactionBatchPeriod", "3s" }
						}
				}};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const NodeConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.Port);
				EXPECT_EQ(0u, config.ApiPort);
				EXPECT_FALSE(config.ShouldAllowAddressReuse);

				EXPECT_EQ(0u, config.MinBlocksPerSyncAttempt);
				EXPECT_EQ(0u, config.MaxBlocksPerSyncAttempt);
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.MinChainBytesPerSyncAttempt);
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
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.MaxPacketDataSize);

				EXPECT_EQ(0u, config.BlockDisruptorSize);
				EXPECT_EQ(0u, config.BlockElementTraceInterval);
				EXPECT_EQ(0u, config.TransactionDisruptorSize);
				EXPECT_EQ(0u, config.TransactionElementTraceInterval);
				EXPECT_EQ(utils::TimeSpan::FromMinutes(0), config.TransactionBatchPeriod);
			}

			static void AssertCustom(const NodeConfiguration& config) {
				// Assert:
				EXPECT_EQ(1234u, config.Port);
				EXPECT_EQ(8888u, config.ApiPort);
				EXPECT_TRUE(config.ShouldAllowAddressReuse);

				EXPECT_EQ(25u, config.MinBlocksPerSyncAttempt);
				EXPECT_EQ(50u, config.MaxBlocksPerSyncAttempt);
				EXPECT_EQ(utils::FileSize::FromMegabytes(1), config.MinChainBytesPerSyncAttempt);
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
				EXPECT_EQ(utils::FileSize::FromMegabytes(10), config.MaxPacketDataSize);

				EXPECT_EQ(1000u, config.BlockDisruptorSize);
				EXPECT_EQ(34u, config.BlockElementTraceInterval);
				EXPECT_EQ(9876u, config.TransactionDisruptorSize);
				EXPECT_EQ(98u, config.TransactionElementTraceInterval);
				EXPECT_EQ(utils::TimeSpan::FromSeconds(3), config.TransactionBatchPeriod);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(NodeConfigurationTests, Node)
}}
