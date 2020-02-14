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

#pragma once
#include "NodeTestUtils.h"
#include "SocketTestUtils.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/net/NodeRequestResult.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace test {

	// region RequestorTestContext

	/// Test context around a brief server requestor.
	template<typename TRequestor>
	struct BriefServerRequestorTestContext {
	public:
		/// Creates a test context with the specified \a timeout around a requestor initialized with \a requestorParam.
		template<typename TRequestorParam>
		BriefServerRequestorTestContext(const utils::TimeSpan& timeout, const TRequestorParam& requestorParam)
				: ClientKeyPair(GenerateKeyPair())
				, pPool(CreateStartedIoThreadPool())
				, ServerKeyPair(GenerateKeyPair())
				, ServerPublicKey(ServerKeyPair.publicKey())
				, pRequestor(std::make_shared<TRequestor>(pPool, ClientKeyPair, CreateSettingsWithTimeout(timeout), requestorParam))
		{}

	public:
		crypto::KeyPair ClientKeyPair;
		std::shared_ptr<thread::IoThreadPool> pPool;

		crypto::KeyPair ServerKeyPair;
		Key ServerPublicKey;

		std::shared_ptr<TRequestor> pRequestor;

	public:
		void waitForActiveConnections(uint32_t numConnections) const {
			WAIT_FOR_VALUE_EXPR(numConnections, pRequestor->numActiveConnections());
		}

	private:
		static net::ConnectionSettings CreateSettingsWithTimeout(const utils::TimeSpan& timeout) {
			auto settings = net::ConnectionSettings();
			settings.Timeout = timeout;
			settings.SslOptions = CreatePacketSocketSslOptions();
			return settings;
		}
	};

	// endregion

	/// Policy for calling request on the requestor.
	struct BriefServerRequestorMemberBeginRequestPolicy {
		template<typename TRequestor, typename TCallback = typename TRequestor::CallbackType>
		static void BeginRequest(TRequestor& requestor, const ionet::Node& requestNode, const TCallback& callback) {
			requestor.beginRequest(requestNode, callback);
		}
	};

	/// Runs a brief server requestor connected test using \a context with the specified response packet (\a pResponsePacket).
	/// \a action is called with the result of beginRequest.
	template<typename TBeginRequestPolicy, typename TTestContext, typename TAction>
	void RunBriefServerRequestorConnectedTest(
			const TTestContext& context,
			const std::shared_ptr<ionet::Packet>& pResponsePacket,
			TAction action) {
		using ResponseType = typename decltype(context.pRequestor)::element_type::ResponseType;

		// Arrange: create a server for connecting
		TcpAcceptor acceptor(context.pPool->ioContext());

		std::shared_ptr<ionet::PacketSocket> pServerSocket;
		SpawnPacketServerWork(acceptor, [&context, &pServerSocket, pResponsePacket](const auto& pSocket) {
			pServerSocket = pSocket;
			net::VerifyClient(pSocket, context.ServerKeyPair, ionet::ConnectionSecurityMode::None, [pResponsePacket, pSocket](
					auto,
					const auto&) {
				// - write the packet if specified
				if (pResponsePacket)
					pSocket->write(ionet::PacketPayload(pResponsePacket), [](auto) {});
			});
		});

		// Act: initiate a request
		std::atomic<size_t> numCallbacks(0);
		std::vector<std::pair<net::NodeRequestResult, ResponseType>> resultPairs;
		auto requestNode = CreateLocalHostNode(context.ServerPublicKey);
		TBeginRequestPolicy::BeginRequest(*context.pRequestor, requestNode, [&numCallbacks, &resultPairs](
				auto result,
				const auto& response) {
			resultPairs.emplace_back(result, response);
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert:
		ASSERT_EQ(1u, resultPairs.size());
		action(*context.pRequestor, resultPairs[0].first, resultPairs[0].second);

		// - no connections remain
		context.waitForActiveConnections(0);
		EXPECT_EQ(0u, context.pRequestor->numActiveConnections());
	}

	/// Runs a brief server requestor connected test using \a context with the specified response packet (\a pResponsePacket).
	/// \a action is called with the result of beginRequest.
	template<typename TTestContext, typename TAction>
	void RunBriefServerRequestorConnectedTest(
			const TTestContext& context,
			const std::shared_ptr<ionet::Packet>& pResponsePacket,
			TAction action) {
		RunBriefServerRequestorConnectedTest<BriefServerRequestorMemberBeginRequestPolicy>(context, pResponsePacket, action);
	}

	/// Asserts that the specified brief server \a requestor indicates a single failed request.
	template<typename TRequestor>
	void AssertBriefServerRequestorFailedConnection(const TRequestor& requestor) {
		// Assert:
		EXPECT_EQ(1u, requestor.numTotalRequests());
		EXPECT_EQ(0u, requestor.numSuccessfulRequests());
	}

	// region RemotePullServer

	/// Remote pull server.
	class RemotePullServer {
	public:
		/// Creates a remote pull server.
		RemotePullServer()
				: m_pPool(test::CreateStartedIoThreadPool(1))
				, m_acceptor(m_pPool->ioContext())
		{}

	public:
		/// Returns \c true if the server is connected.
		bool hasConnection() const {
			return !!serverSocket();
		}

	protected:
		void prepareValidResponse(const crypto::KeyPair& partnerKeyPair, const std::shared_ptr<ionet::Packet>& pResponsePacket) {
			test::SpawnPacketServerWork(m_acceptor, [this, &partnerKeyPair, pResponsePacket](const auto& pSocket) {
				this->setServerSocket(pSocket);

				auto securityMode = ionet::ConnectionSecurityMode::None;
				net::VerifyClient(pSocket, partnerKeyPair, securityMode, [pResponsePacket, pSocket](auto, const auto&) {
					// write the packet
					pSocket->write(ionet::PacketPayload(pResponsePacket), [](auto) {});
				});
			});
		}

	public:
		/// Spawns server work but does not respond to any request.
		void prepareNoResponse() {
			test::SpawnPacketServerWork(m_acceptor, [this](const auto& pSocket) {
				this->setServerSocket(pSocket);
			});
		}

		/// Closes the socket.
		void close() {
			serverSocket()->close();
		}

	private:
		std::shared_ptr<ionet::PacketSocket> serverSocket() const {
			return std::atomic_load(&m_pServerSocket);
		}

		void setServerSocket(const std::shared_ptr<ionet::PacketSocket>& pServerSocket) {
			std::atomic_store(&m_pServerSocket, pServerSocket);
		}

	private:
		std::unique_ptr<thread::IoThreadPool> m_pPool;
		test::TcpAcceptor m_acceptor;

		std::shared_ptr<ionet::PacketSocket> m_pServerSocket;
	};

	// endregion
}}
