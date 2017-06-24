#include "NetworkConnections.h"
#include "tools/ToolKeys.h"
#include "tools/ToolMain.h"
#include "tools/ToolUtils.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"

namespace catapult { namespace tools {

	NetworkConnections::NetworkConnections(const config::LocalNodeConfiguration& config)
			: m_pPool(CreateStartedThreadPool(std::thread::hardware_concurrency()))
			, m_clientKeyPair(GenerateRandomKeyPair())
			, m_config(config) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = model::NetworkIdentifier::Mijin_Test;
		m_pPacketWriters = net::CreatePacketWriters(m_pPool, m_clientKeyPair, settings);
	}

	NetworkConnections::~NetworkConnections() {
		shutdown();
	}

	thread::future<bool> NetworkConnections::connectAll() {
		std::vector<thread::future<bool>> futures;
		futures.reserve(m_config.Peers.size());

		for (const auto& node : m_config.Peers) {
			auto pPromise = std::make_shared<thread::promise<bool>>();
			futures.push_back(pPromise->get_future());

			m_pPacketWriters->connect(node, [node, pPromise](auto connectResult) {
				pPromise->set_value(true);
				CATAPULT_LOG(info) << "connection attempt to " << node << " completed with " << connectResult;
			});
		}

		return thread::when_all(std::move(futures)).then([](const auto&) { return true; });
	}

	ionet::NodePacketIoPair NetworkConnections::pickOne() const {
		for (auto tryCount = 0u; tryCount < 5u; ++tryCount) {
			auto ioPair = m_pPacketWriters->pickOne(utils::TimeSpan::FromMilliseconds(3000));
			if (ioPair)
				return ioPair;

			std::this_thread::yield();
		}

		return ionet::NodePacketIoPair();
	}

	void NetworkConnections::shutdown() {
		if (m_pPool) {
			m_pPacketWriters->shutdown();
			m_pPool->join();
		}
	}
}}
