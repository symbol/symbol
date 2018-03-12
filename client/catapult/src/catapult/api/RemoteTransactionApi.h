#pragma once
#include "catapult/model/RangeTypes.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// An api for retrieving transaction information from a remote node.
	class RemoteTransactionApi {
	public:
		virtual ~RemoteTransactionApi() {}

	public:
		/// Gets all unconfirmed transactions from the remote excluding those with hashes in \a knownShortHashes.
		virtual thread::future<model::TransactionRange> unconfirmedTransactions(model::ShortHashRange&& knownShortHashes) const = 0;
	};

	/// Creates a transaction api for interacting with a remote node with the specified \a io
	/// and transaction \a registry composed of supported transactions.
	std::unique_ptr<RemoteTransactionApi> CreateRemoteTransactionApi(ionet::PacketIo& io, const model::TransactionRegistry& registry);
}}
