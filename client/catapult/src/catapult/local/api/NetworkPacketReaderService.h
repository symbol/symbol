#pragma once
#include "catapult/handlers/TransactionHandlers.h"
#include "catapult/local/BasicNetworkPacketReaderService.h"

namespace catapult { namespace local { namespace api {

	/// A service for handling api client requests (used for reading packets from tcp).
	class NetworkPacketReaderService : public BasicNetworkPacketReaderService {
	public:
		/// Creates a service around \a keyPair, \a stateRef and \a transactionRegistry.
		/// Pushed entities are forwarded to \a transactionRangeConsumer.
		NetworkPacketReaderService(
				const crypto::KeyPair& keyPair,
				const LocalNodeStateConstRef& stateRef,
				const model::TransactionRegistry& transactionRegistry,
				const handlers::TransactionRangeHandler& transactionRangeConsumer);

	private:
		void registerHandlers(ionet::ServerPacketHandlers& serverPacketHandlers, const LocalNodeStateConstRef& stateRef) override;

	private:
		// state
		const model::TransactionRegistry& m_transactionRegistry;
		handlers::TransactionRangeHandler m_transactionRangeConsumer;
	};
}}}
