#include "NetworkConnections.h"
#include "ToolKeys.h"
#include "ToolThreadUtils.h"
#include "Waits.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"

namespace catapult { namespace tools {

	NetworkConnections::NetworkConnections(const std::vector<ionet::Node>& nodes)
			: m_pPool(CreateStartedThreadPool(std::thread::hardware_concurrency()))
			, m_clientKeyPair(GenerateRandomKeyPair())
			, m_nodes(nodes) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = model::NetworkIdentifier::Mijin_Test;
		m_pPacketWriters = net::CreatePacketWriters(m_pPool, m_clientKeyPair, settings);
	}

	NetworkConnections::~NetworkConnections() {
		shutdown();
	}

	size_t NetworkConnections::numActiveConnections() const {
		return m_pPacketWriters->numActiveConnections();
	}

	size_t NetworkConnections::numActiveWriters() const {
		return m_pPacketWriters->numActiveWriters();
	}

	size_t NetworkConnections::numAvailableWriters() const {
		return m_pPacketWriters->numAvailableWriters();
	}

	thread::future<bool> NetworkConnections::connectAll() {
		std::vector<thread::future<bool>> futures;
		futures.reserve(m_nodes.size());

		for (const auto& node : m_nodes) {
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

	Height GetHeight(const NetworkConnections& connections) {
		auto ioPair = connections.pickOne();
		if (!ioPair)
			CATAPULT_THROW_RUNTIME_ERROR("no connection to network available");

		auto pChainApi = api::CreateRemoteChainApiWithoutRegistry(*ioPair.io());
		return pChainApi->chainInfo().get().Height;
	}

	bool WaitForBlocks(const NetworkConnections& connections, size_t numBlocks) {
		try {
			CATAPULT_LOG(info) << "waiting for " << numBlocks << " blocks...";
			auto currentHeight = GetHeight(connections);
			auto endHeight = currentHeight + Height(numBlocks);
			CATAPULT_LOG(info) << "waiting for block " << endHeight << ", current block is " << currentHeight;
			while (currentHeight < endHeight) {
				Sleep();
				Height oldHeight = currentHeight;
				currentHeight = GetHeight(connections);
				if (currentHeight != oldHeight)
					CATAPULT_LOG(info) << "block " << currentHeight << " was harvested";
			}

			return true;
		} catch (const std::exception& e) {
			CATAPULT_LOG(error) << "exception thrown while waiting for new blocks: " << e.what();
			throw;
		}
	}
}}
