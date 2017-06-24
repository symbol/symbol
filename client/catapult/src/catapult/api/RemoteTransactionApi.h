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
		/// Gets all unconfirmed transactions from the remote whose short hashes are not in \a knownShortHashes.
		virtual thread::future<model::TransactionRange> unconfirmedTransactions(
				model::ShortHashRange&& knownShortHashes) const = 0;
	};

	/// Creates a transaction api for interacting with a remote node with the specified io (\a pIo)
	/// and transaction registry (\a pRegistry) composed of supported transactions.
	std::unique_ptr<RemoteTransactionApi> CreateRemoteTransactionApi(
			const std::shared_ptr<ionet::PacketIo>& pIo,
			const std::shared_ptr<const model::TransactionRegistry>& pRegistry);
}}
