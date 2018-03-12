#pragma once
#include "catapult/ionet/NodeRoles.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include <unordered_set>

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

		/// \c true if a single thread pool should be used, \c false if multiple thread pools should be used.
		bool ShouldUseSingleThreadPool;

		/// The maximum number of blocks per sync attempt.
		uint32_t MaxBlocksPerSyncAttempt;

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

		/// The initial socket working buffer size (socket reads will attempt to read buffers of roughly this size).
		utils::FileSize SocketWorkingBufferSize;

		/// The socket working buffer sensitivity (lower values will cause memory to be more aggressively reclaimed).
		/// \note \c 0 will disable memory reclamation.
		uint32_t SocketWorkingBufferSensitivity;

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

		/// \c true if the process should terminate when any dispatcher is full.
		bool ShouldAbortWhenDispatcherIsFull;

		/// \c true if all dispatcher inputs should be audited.
		bool ShouldAuditDispatcherInputs;

		/// \c true if all transaction addresses should be extracted during dispatcher processing.
		bool ShouldPrecomputeTransactionAddresses;

		/// The named extensions to enable.
		std::unordered_set<std::string> Extensions;

	public:
		/// Local node configuration.
		struct LocalSubConfiguration {
			/// The node host (leave empty to auto-detect IP).
			std::string Host;

			/// The node friendly name (leave empty to use address).
			std::string FriendlyName;

			/// The node version.
			uint32_t Version;

			/// The node roles.
			ionet::NodeRoles Roles;
		};

	public:
		/// Local node configuration.
		LocalSubConfiguration Local;

	public:
		/// Connections configuration.
		struct ConnectionsSubConfiguration {
			/// The maximum number of active connections.
			uint16_t MaxConnections;

			/// The maximum connection age.
			uint16_t MaxConnectionAge;
		};

		/// Incoming connections configuration.
		struct IncomingConnectionsSubConfiguration : public ConnectionsSubConfiguration {
			/// The maximum size of the pending connections queue.
			uint16_t BacklogSize;
		};

	public:
		/// Outgoing connections configuration.
		ConnectionsSubConfiguration OutgoingConnections;

		/// Incoming connections configuration.
		IncomingConnectionsSubConfiguration IncomingConnections;

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
