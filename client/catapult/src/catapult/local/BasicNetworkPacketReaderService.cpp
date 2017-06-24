#include "BasicNetworkPacketReaderService.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/local/NetworkUtils.h"
#include "catapult/net/PacketReaders.h"

namespace catapult { namespace local {

	BasicNetworkPacketReaderService::BasicNetworkPacketReaderService(
			const crypto::KeyPair& keyPair,
			const LocalNodeStateConstRef& stateRef)
			: m_keyPair(keyPair)
			, m_stateRef(stateRef)
	{}

	size_t BasicNetworkPacketReaderService::numActiveReaders() const {
		return GetStatsValue(m_pReaders.lock(), &net::PacketReaders::numActiveReaders);
	}

	void BasicNetworkPacketReaderService::boot(thread::MultiServicePool& pool) {
		ionet::ServerPacketHandlers serverPacketHandlers;
		registerHandlers(serverPacketHandlers, m_stateRef);

		const auto& config = m_stateRef.Config;
		auto connectionSettings = GetConnectionSettings(config);
		auto pPeerServiceGroup = pool.pushServiceGroup("p2p");
		m_pReaders = pPeerServiceGroup->pushService(net::CreatePacketReaders, serverPacketHandlers, m_keyPair, connectionSettings);
		m_pPeerServer = BootServer(*pPeerServiceGroup, config.Node.Port, config, *m_pReaders.lock());
	}
}}
