#pragma once
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Node configuration settings.
	struct NodeConfiguration {
	public:
		/// The server port.
		unsigned short Port;

		/// The server api port.
		unsigned short ApiPort;

		/// \c true if the server should reuse ports already in use.
		bool ShouldAllowAddressReuse;

		/// The minimum number of blocks per sync attempt.
		uint32_t MinBlocksPerSyncAttempt;

		/// The maximum number of blocks per sync attempt.
		uint32_t MaxBlocksPerSyncAttempt;

		/// The minimum chain bytes per sync attempt.
		utils::FileSize MinChainBytesPerSyncAttempt;

		/// The maximum chain bytes per sync attempt.
		utils::FileSize MaxChainBytesPerSyncAttempt;

		/// The duration of a transaction in the short lived cache.
		utils::TimeSpan ShortLivedCacheTransactionDuration;

		/// The duration of a block in the short lived cache.
		utils::TimeSpan ShortLivedCacheBlockDuration;

		/// The time between short lived cache pruning.
		utils::TimeSpan ShortLivedCachePruneInterval;

		/// The maximum size of a short lived cache.
		uint32_t ShortLivedCacheMaxSize;

		/// The maximum size of an unconfirmed transactions response.
		utils::FileSize UnconfirmedTransactionsCacheMaxResponseSize;

		/// The maximum size of the unconfirmed transactions cache.
		uint32_t UnconfirmedTransactionsCacheMaxSize;

		/// The timeout for connecting to a peer.
		utils::TimeSpan ConnectTimeout;

		/// The timeout for syncing with a peer.
		utils::TimeSpan SyncTimeout;

		/// The socket working buffer size (socket reads will attempt to read buffers of this size).
		utils::FileSize SocketWorkingBufferSize;

		/// The maximum packet data size.
		utils::FileSize MaxPacketDataSize;

		/// The size of the block disruptor circular buffer.
		uint32_t BlockDisruptorSize;

		/// The multiple of elements at which a block element should be traced through queue and completion.
		uint32_t BlockElementTraceInterval;

		/// The size of the transaction disruptor circular buffer.
		uint32_t TransactionDisruptorSize;

		/// The multiple of elements at which a transaction element should be traced through queue and completion.
		uint32_t TransactionElementTraceInterval;

		/// The period of time over which transactions should be batched for processing.
		utils::TimeSpan TransactionBatchPeriod;

	private:
		NodeConfiguration() = default;

	public:
		/// Creates an uninitialized node configuration.
		static NodeConfiguration Uninitialized();

	public:
		/// Loads a node configuration from \a bag.
		static NodeConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
