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

#pragma once
#include "catapult/ionet/NodeRoles.h"
#include "catapult/ionet/NodeVersion.h"
#include "catapult/model/TransactionSelectionStrategy.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include <unordered_set>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Node configuration settings.
	struct NodeConfiguration {
	public:
		/// Server port.
		unsigned short Port;

		/// Maximum number of incoming connections per identity over primary port.
		uint32_t MaxIncomingConnectionsPerIdentity;

		/// \c true if the server should reuse ports already in use.
		bool EnableAddressReuse;

		/// \c true if a single thread pool should be used, \c false if multiple thread pools should be used.
		bool EnableSingleThreadPool;

		/// \c true if cache data should be saved in a database.
		bool EnableCacheDatabaseStorage;

		/// \c true if temporary sync files should be automatically cleaned up.
		/// \note This should be \c false if broker process is running.
		bool EnableAutoSyncCleanup;

		/// Maximum number of payloads to store in each file database disk file.
		/// \note This is recommended to be a factor of 10000.
		uint32_t FileDatabaseBatchSize;

		/// \c true if transaction spam throttling should be enabled.
		bool EnableTransactionSpamThrottling;

		/// Maximum fee that will boost a transaction through the spam throttle when spam throttling is enabled.
		Amount TransactionSpamThrottlingMaxBoostFee;

		/// Maximum number of hashes per sync attempt.
		uint32_t MaxHashesPerSyncAttempt;

		/// Maximum number of blocks per sync attempt.
		uint32_t MaxBlocksPerSyncAttempt;

		/// Maximum chain bytes per sync attempt.
		utils::FileSize MaxChainBytesPerSyncAttempt;

		/// Duration of a transaction in the short lived cache.
		utils::TimeSpan ShortLivedCacheTransactionDuration;

		/// Duration of a block in the short lived cache.
		utils::TimeSpan ShortLivedCacheBlockDuration;

		/// Time between short lived cache pruning.
		utils::TimeSpan ShortLivedCachePruneInterval;

		/// Maximum size of a short lived cache.
		uint32_t ShortLivedCacheMaxSize;

		/// Minimum fee multiplier of transactions to propagate and include in blocks.
		BlockFeeMultiplier MinFeeMultiplier;

		/// Transaction pulls will only be initiated when the timestamp of the last block in the local chain is within this value
		/// of the network time.
		utils::TimeSpan MaxTimeBehindPullTransactionsStart;

		/// Transaction selection strategy used for syncing and harvesting unconfirmed transactions.
		model::TransactionSelectionStrategy TransactionSelectionStrategy;

		/// Maximum size of an unconfirmed transactions response.
		utils::FileSize UnconfirmedTransactionsCacheMaxResponseSize;

		/// Maximum size of the unconfirmed transactions cache.
		utils::FileSize UnconfirmedTransactionsCacheMaxSize;

		/// Timeout for connecting to a peer.
		utils::TimeSpan ConnectTimeout;

		/// Timeout for syncing with a peer.
		utils::TimeSpan SyncTimeout;

		/// Initial socket working buffer size (socket reads will attempt to read buffers of roughly this size).
		utils::FileSize SocketWorkingBufferSize;

		/// Socket working buffer sensitivity (lower values will cause memory to be more aggressively reclaimed).
		/// \note \c 0 will disable memory reclamation.
		uint32_t SocketWorkingBufferSensitivity;

		/// Maximum packet data size.
		utils::FileSize MaxPacketDataSize;

		/// Number of slots in the block disruptor circular buffer.
		uint32_t BlockDisruptorSlotCount;

		/// Maximum memory of all elements in the block disruptor circular buffer.
		utils::FileSize BlockDisruptorMaxMemorySize;

		/// Multiple of elements at which a block element should be traced through queue and completion.
		uint32_t BlockElementTraceInterval;

		/// Number of slots in the transaction disruptor circular buffer.
		uint32_t TransactionDisruptorSlotCount;

		/// Maximum memory of all elements in the transaction disruptor circular buffer.
		utils::FileSize TransactionDisruptorMaxMemorySize;

		/// Multiple of elements at which a transaction element should be traced through queue and completion.
		uint32_t TransactionElementTraceInterval;

		/// \c true if the process should terminate when any dispatcher is full.
		bool EnableDispatcherAbortWhenFull;

		/// \c true if all dispatcher inputs should be audited.
		bool EnableDispatcherInputAuditing;

		/// Maximum number of nodes to track in memory.
		uint32_t MaxTrackedNodes;

		/// Minimum supported version of partner nodes.
		ionet::NodeVersion MinPartnerNodeVersion;

		/// Maximum supported version of partner nodes.
		ionet::NodeVersion MaxPartnerNodeVersion;

		/// Trusted hosts that are allowed to execute protected API calls on this node.
		std::unordered_set<std::string> TrustedHosts;

		/// Networks that should be treated as local.
		std::unordered_set<std::string> LocalNetworks;

		/// Network interface on which to listen.
		std::string ListenInterface;

	public:
		/// Cache database configuration.
		struct CacheDatabaseSubConfiguration {
			/// \c true if operational statistics should be captured and logged.
			bool EnableStatistics;

			/// Maximum number of open files.
			uint32_t MaxOpenFiles;

			/// Maximum number of log files.
			uint32_t MaxLogFiles;

			/// Maximum log file size.
			utils::FileSize MaxLogFileSize;

			/// Maximum number of background threads.
			uint32_t MaxBackgroundThreads;

			/// Maximum number of threads that will concurrently perform a compaction.
			uint32_t MaxSubcompactionThreads;

			/// Block cache size.
			/// \note Optimizes for point lookup when nonzero.
			utils::FileSize BlockCacheSize;

			/// Memtable memory budget.
			/// \note Optimizes level style compaction when nonzero.
			utils::FileSize MemtableMemoryBudget;

			/// Maximum write batch size.
			utils::FileSize MaxWriteBatchSize;
		};

	public:
		/// Cache database configuration.
		CacheDatabaseSubConfiguration CacheDatabase;

	public:
		/// Local node configuration.
		struct LocalSubConfiguration {
			/// Node host (leave empty to auto-detect IP).
			std::string Host;

			/// Node friendly name (leave empty to use address).
			std::string FriendlyName;

			/// Node version.
			ionet::NodeVersion Version;

			/// Node roles.
			ionet::NodeRoles Roles;
		};

	public:
		/// Local node configuration.
		LocalSubConfiguration Local;

	public:
		/// Connections configuration.
		struct ConnectionsSubConfiguration {
			/// Maximum number of active connections.
			uint16_t MaxConnections;

			/// Maximum connection age.
			uint16_t MaxConnectionAge;

			/// Maximum connection ban age.
			uint16_t MaxConnectionBanAge;

			/// Number of consecutive connection failures before a connection is banned.
			uint16_t NumConsecutiveFailuresBeforeBanning;
		};

		/// Incoming connections configuration.
		struct IncomingConnectionsSubConfiguration : public ConnectionsSubConfiguration {
			/// Maximum size of the pending connections queue.
			uint16_t BacklogSize;
		};

	public:
		/// Outgoing connections configuration.
		ConnectionsSubConfiguration OutgoingConnections;

		/// Incoming connections configuration.
		IncomingConnectionsSubConfiguration IncomingConnections;

	public:
		/// Banning configuration.
		struct BanningSubConfiguration {
			/// Default duration for banning.
			utils::TimeSpan DefaultBanDuration;

			/// Maximum duration for banning.
			utils::TimeSpan MaxBanDuration;

			/// Duration to keep account in container after the ban expired.
			utils::TimeSpan KeepAliveDuration;

			/// Maximum number of banned nodes.
			uint32_t MaxBannedNodes;

			/// Number of read rate monitoring buckets (\c 0 to disable read rate monitoring).
			uint16_t NumReadRateMonitoringBuckets;

			/// Duration of each read rate monitoring bucket.
			utils::TimeSpan ReadRateMonitoringBucketDuration;

			/// Maximum size allowed during full read rate monitoring period.
			utils::FileSize MaxReadRateMonitoringTotalSize;

			/// Minimum number of transaction failures to trigger a ban.
			uint32_t MinTransactionFailuresCountForBan;

			/// Minimum percentage of transaction failures to trigger a ban.
			uint32_t MinTransactionFailuresPercentForBan;
		};

	public:
		/// Bannning configuration
		BanningSubConfiguration Banning;

	private:
		NodeConfiguration() = default;

	public:
		/// Creates an uninitialized node configuration.
		static NodeConfiguration Uninitialized();

	public:
		/// Loads a node configuration from \a bag.
		static NodeConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	/// Returns \c true when \a host is contained in \a localNetworks.
	bool IsLocalHost(const std::string& host, const std::unordered_set<std::string>& localNetworks);
}}
