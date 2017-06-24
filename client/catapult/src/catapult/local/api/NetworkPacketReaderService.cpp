#include "NetworkPacketReaderService.h"

namespace catapult { namespace local { namespace api {

	NetworkPacketReaderService::NetworkPacketReaderService(
			const crypto::KeyPair& keyPair,
			const LocalNodeStateConstRef& stateRef,
			const model::TransactionRegistry& transactionRegistry,
			const handlers::TransactionRangeHandler& transactionRangeConsumer)
			: BasicNetworkPacketReaderService(keyPair, stateRef)
			, m_transactionRegistry(transactionRegistry)
			, m_transactionRangeConsumer(transactionRangeConsumer)
	{}

	void NetworkPacketReaderService::registerHandlers(
			ionet::ServerPacketHandlers& serverPacketHandlers,
			const LocalNodeStateConstRef&) {
		handlers::RegisterPushTransactionsHandler(serverPacketHandlers, m_transactionRegistry, m_transactionRangeConsumer);
	}
}}}
