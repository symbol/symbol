#pragma once
#include "catapult/api/LocalChainApi.h"
#include "catapult/chain/ChainSynchronizer.h"
#include "catapult/thread/Scheduler.h"

namespace catapult {
	namespace config { class LocalNodeConfiguration; }
	namespace io { class BlockStorageCache; }
	namespace ionet { struct Node; }
	namespace model { class TransactionRegistry; }
	namespace net { class PacketWriters; }
}

namespace catapult { namespace local {

	/// Creates a connect peers task around \a packetWriters for the peers specified in \a peers.
	thread::Task CreateConnectPeersTask(const std::vector<ionet::Node>& peers, net::PacketWriters& packetWriters);

	/// Creates a synchronizer task around \a transactionRegistry and \a packetWriters using \a config and \a storage with handlers
	/// \a chainScoreSupplier, shortHashesSupplier, \a blockRangeConsumer and \a transactionRangeConsumer.
	thread::Task CreateSynchronizerTask(
			const config::LocalNodeConfiguration& config,
			const io::BlockStorageCache& storage,
			const model::TransactionRegistry& transactionRegistry,
			net::PacketWriters& packetWriters,
			const catapult::api::ChainScoreSupplier& chainScoreSupplier,
			const chain::ShortHashesSupplier& shortHashesSupplier,
			const chain::CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer,
			const chain::TransactionRangeConsumerFunc& transactionRangeConsumer);
}}
