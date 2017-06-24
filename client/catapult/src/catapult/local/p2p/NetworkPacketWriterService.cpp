#include "NetworkPacketWriterService.h"
#include "catapult/local/BroadcastUtils.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/local/NetworkUtils.h"
#include "catapult/net/PacketWriters.h"

namespace catapult { namespace local { namespace p2p {

	NetworkPacketWriterService::NetworkPacketWriterService(
			const crypto::KeyPair& keyPair,
			const config::LocalNodeConfiguration& config)
			: m_keyPair(keyPair)
			, m_config(config)
	{}

	size_t NetworkPacketWriterService::numActiveWriters() const {
		return GetStatsValue(m_pWriters.lock(), &net::PacketWriters::numActiveWriters);
	}

	size_t NetworkPacketWriterService::numActiveBroadcastWriters() const {
		return GetStatsValue(m_pBroadcastWriters.lock(), &net::PacketWriters::numActiveWriters);
	}

	net::PacketWriters& NetworkPacketWriterService::packetWriters() {
		return *m_pWriters.lock();
	}

	template<typename TSink>
	TSink NetworkPacketWriterService::createPushEntitySink() const {
		return [this](const auto& entities) {
			auto payload = CreateBroadcastPayload(entities);
			m_pWriters.lock()->broadcast(payload);
			m_pBroadcastWriters.lock()->broadcast(payload);
		};
	}

	NewBlockSink NetworkPacketWriterService::createNewBlockSink() {
		return createPushEntitySink<NewBlockSink>();
	}

	SharedNewTransactionsSink NetworkPacketWriterService::createNewTransactionsSink() {
		return createPushEntitySink<SharedNewTransactionsSink>();
	}

	void NetworkPacketWriterService::boot(thread::MultiServicePool& pool) {
		auto connectionSettings = GetConnectionSettings(m_config);
		m_pWriters = pool.pushServiceGroup("writers")->pushService(net::CreatePacketWriters, m_keyPair, connectionSettings);

		auto pApiServiceGroup = pool.pushServiceGroup("api");
		m_pBroadcastWriters = pApiServiceGroup->pushService(net::CreatePacketWriters, m_keyPair, connectionSettings);
		m_pApiServer = BootServer(*pApiServiceGroup, m_config.Node.ApiPort, m_config, *m_pBroadcastWriters.lock());
	}
}}}
