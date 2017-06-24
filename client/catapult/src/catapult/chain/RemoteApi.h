#pragma once
#include "catapult/api/RemoteChainApi.h"
#include "catapult/api/RemoteTransactionApi.h"

namespace catapult {
	namespace ionet { class PacketIo; }
	namespace model { class TransactionRegistry; }
}

namespace catapult { namespace chain {

	/// An aggregate api composed of a chain api and a transaction api.
	struct RemoteApi {
		/// Creates a remote api.
		RemoteApi() = default;

		/// Creates a remote api around a packet io (\a pIo) and a transaction registry (\a pTransactionRegistry).
		RemoteApi(
				const std::shared_ptr<ionet::PacketIo>& pIo,
				const std::shared_ptr<const model::TransactionRegistry>& pTransactionRegistry)
				: pChainApi(api::CreateRemoteChainApi(pIo, pTransactionRegistry))
				, pTransactionApi(api::CreateRemoteTransactionApi(pIo, pTransactionRegistry))
		{}

		/// The remote chain api.
		std::shared_ptr<api::RemoteChainApi> pChainApi;

		/// The remote transaction api.
		std::shared_ptr<api::RemoteTransactionApi> pTransactionApi;
	};
}}
