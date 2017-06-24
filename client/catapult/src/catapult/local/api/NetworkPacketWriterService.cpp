#include "NetworkPacketWriterService.h"
#include "catapult/local/BroadcastUtils.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/local/NetworkUtils.h"
#include "catapult/net/PacketWriters.h"

namespace catapult { namespace local { namespace api {

	NetworkPacketWriterService::NetworkPacketWriterService(
			const crypto::KeyPair& keyPair,
			const config::LocalNodeConfiguration& config)
			: m_keyPair(keyPair)
			, m_config(config)
	{}

	size_t NetworkPacketWriterService::numActiveWriters() const {
		return GetStatsValue(m_pWriters.lock(), &net::PacketWriters::numActiveWriters);
	}

	net::PacketWriters& NetworkPacketWriterService::packetWriters() {
		return *m_pWriters.lock();
	}

	template<typename TSink>
	TSink NetworkPacketWriterService::createPushEntitySink() const {
		return [this](const auto& entities) {
			auto payload = CreateBroadcastPayload(entities);
			m_pWriters.lock()->broadcast(payload);
		};
	}

	SharedNewTransactionsSink NetworkPacketWriterService::createNewTransactionsSink() {
		return createPushEntitySink<SharedNewTransactionsSink>();
	}

	void NetworkPacketWriterService::boot(thread::MultiServicePool& pool) {
		m_pWriters = pool.pushServiceGroup("writers")->pushService(net::CreatePacketWriters, m_keyPair, GetConnectionSettings(m_config));
	}
}}}
