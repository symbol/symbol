#pragma once
#include "partialtransaction/src/PtTypes.h"
#include "catapult/cache/ShortHashPair.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// An api for retrieving partial transaction information from a remote node.
	class RemotePtApi {
	public:
		virtual ~RemotePtApi() {}

	public:
		/// Gets all partial transaction infos from the remote excluding those with all hashes in \a knownShortHashPairs.
		virtual thread::future<partialtransaction::CosignedTransactionInfos> transactionInfos(
				cache::ShortHashPairRange&& knownShortHashPairs) const = 0;
	};

	/// Creates a partial transaction api for interacting with a remote node with the specified \a io
	/// and transaction \a registry composed of supported transactions.
	std::unique_ptr<RemotePtApi> CreateRemotePtApi(ionet::PacketIo& io, const model::TransactionRegistry& registry);
}}
