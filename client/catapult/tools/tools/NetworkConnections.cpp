/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
			, m_pClientKeyPair(std::make_unique<crypto::KeyPair>(GenerateRandomKeyPair()))
			, m_nodes(nodes)
			, m_pSpinLock(std::make_unique<utils::SpinLock>()) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = model::NetworkIdentifier::Mijin_Test;
		m_pPacketWriters = net::CreatePacketWriters(m_pPool, *m_pClientKeyPair, settings);
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
		utils::SpinLockGuard guard(*m_pSpinLock);
		return connectAll(m_nodes);
	}

	thread::future<bool> NetworkConnections::connectDisconnected() {
		utils::SpinLockGuard guard(*m_pSpinLock);
		return connectAll({ m_disconnectedNodes.cbegin(), m_disconnectedNodes.cend() });
	}

	thread::future<bool> NetworkConnections::connectAll(const std::vector<ionet::Node>& nodes) {
		if (nodes.empty())
			return thread::make_ready_future(true);

		std::vector<thread::future<bool>> futures;
		futures.reserve(nodes.size());

		for (const auto& node : nodes) {
			CATAPULT_LOG(info) << "attempting to connect to node " << node;
			auto pPromise = std::make_shared<thread::promise<bool>>();
			futures.push_back(pPromise->get_future());

			m_pPacketWriters->connect(node, [this, node, pPromise](const auto& connectResult) {
				auto connectCode = connectResult.Code;
				if (net::PeerConnectCode::Accepted == connectCode || net::PeerConnectCode::Already_Connected == connectCode) {
					CATAPULT_LOG(warning) << "removing node " << node << " from set of disconnected nodes";
					this->removeDisconnected(node);
				} else {
					this->addDisconnected(node);
				}

				CATAPULT_LOG(info) << "connection attempt to " << node << " completed with " << connectCode;
				pPromise->set_value(true);
			});
		}

		return thread::when_all(std::move(futures)).then([](const auto&) { return true; });
	}

	ionet::NodePacketIoPair NetworkConnections::pickOne() const {
		for (auto tryCount = 0u; tryCount < 5; ++tryCount) {
			auto ioPair = m_pPacketWriters->pickOne(utils::TimeSpan::FromMilliseconds(3000));
			if (ioPair)
				return ioPair;

			std::this_thread::yield();
		}

		return ionet::NodePacketIoPair();
	}

	void NetworkConnections::addDisconnected(const ionet::Node& node) {
		utils::SpinLockGuard guard(*m_pSpinLock);
		m_disconnectedNodes.emplace(node);
	}

	void NetworkConnections::removeDisconnected(const ionet::Node& node) {
		utils::SpinLockGuard guard(*m_pSpinLock);
		m_disconnectedNodes.erase(node);
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
