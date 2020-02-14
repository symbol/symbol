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
#include "catapult/api/RemoteChainApi.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/net/ServerConnector.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/test/other/RemoteApiFactory.h"

namespace catapult { namespace test {

	// region ExternalSourceConnection

	/// Represents an external source connection.
	class ExternalSourceConnection {
	public:
		/// Creates an external source connection to default local node.
		ExternalSourceConnection() : ExternalSourceConnection(CreateLocalHostNode(LoadServerKeyPair().publicKey(), GetLocalHostPort()))
		{}

		/// Creates an external source connection to specified local \a node.
		explicit ExternalSourceConnection(const ionet::Node& node)
				: m_pPool(CreateStartedIoThreadPool(1))
				, m_clientKeyPair(crypto::KeyPair::FromPrivate(GenerateRandomPrivateKey()))
				, m_pConnector(net::CreateServerConnector(m_pPool, m_clientKeyPair, CreateConnectionSettings(), "external source"))
				, m_localNode(node)
		{}

	public:
		std::shared_ptr<ionet::PacketIo> io() const {
			return m_pIo;
		}

	public:
		/// Connects to the local node and calls \a onConnect on completion.
		void connect(const consumer<const std::shared_ptr<ionet::PacketSocket>&>& onConnect) {
			m_pConnector->connect(m_localNode, [&pIo = m_pIo, onConnect](auto connectCode, const auto& socketInfo) {
				// save pIo in a member to tie the lifetime of the connection to the lifetime of the owning ExternalSourceConnection
				pIo = socketInfo.socket();
				if (net::PeerConnectCode::Accepted == connectCode)
					onConnect(socketInfo.socket());
			});
		}

		/// Connects to the local node and calls \a onConnect on completion.
		void apiCall(const consumer<const std::shared_ptr<api::RemoteChainApi>&>& onConnect) {
			connect([onConnect](const auto& pPacketIo) {
				auto pRemoteApi = CreateLifetimeExtendedApi(
						api::CreateRemoteChainApi,
						pPacketIo,
						model::NodeIdentity(),
						CreateTransactionRegistry());
				onConnect(pRemoteApi);
			});
		}

	private:
		static model::TransactionRegistry CreateTransactionRegistry();

	private:
		std::shared_ptr<thread::IoThreadPool> m_pPool;
		crypto::KeyPair m_clientKeyPair;
		std::shared_ptr<net::ServerConnector> m_pConnector;
		ionet::Node m_localNode;

		std::shared_ptr<ionet::PacketIo> m_pIo;
	};

	// endregion

	// region boot

	/// Waits for \a context to be booted.
	template<typename TTestContext>
	void WaitForBoot(const TTestContext& context) {
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveWriters);
	}

	// endregion

	// region push

	/// Pushes \a payload to \a connection.
	std::shared_ptr<ionet::PacketIo> PushPayload(ExternalSourceConnection& connection, const ionet::PacketPayload& payload);

	/// Pushes \a pEntity as a packet of \a type to \a connection.
	template<typename TEntity>
	std::shared_ptr<ionet::PacketIo> PushEntity(
			ExternalSourceConnection& connection,
			ionet::PacketType type,
			const std::shared_ptr<TEntity>& pEntity) {
		return PushPayload(connection, ionet::PacketPayloadFactory::FromEntity(type, pEntity));
	}

	/// Pushes \a entities as a packet of \a type to \a connection.
	template<typename TEntity>
	std::shared_ptr<ionet::PacketIo> PushEntities(
			ExternalSourceConnection& connection,
			ionet::PacketType type,
			const std::vector<std::shared_ptr<TEntity>>& entities) {
		return PushPayload(connection, ionet::PacketPayloadFactory::FromEntities(type, entities));
	}

	/// Pushes a valid block to \a connection.
	std::shared_ptr<ionet::PacketIo> PushValidBlock(ExternalSourceConnection& connection);

	/// Pushes a valid transaction to \a connection.
	std::shared_ptr<ionet::PacketIo> PushValidTransaction(ExternalSourceConnection& connection);

	// endregion

	// region pull

	/// Asserts that a response can be retrieved for the specified traits using \a context and then calls \a handler.
	template<typename TApiTraits, typename TTestContext, typename THandler>
	void AssertCanGetResponse(TTestContext&& context, THandler handler) {
		// Arrange:
		WaitForBoot(context);

		// Act:
		std::atomic_bool isReadFinished(false);
		bool isExceptionRaised = false;
		typename TApiTraits::ResultType result;
		ExternalSourceConnection connection;
		connection.apiCall([&](const auto& pRemoteChainApi) {
			TApiTraits::InitiateValidRequest(*pRemoteChainApi).then([&, pRemoteChainApi](auto&& future) {
				try {
					result = std::move(future.get());
				} catch (const catapult_runtime_error& ex) {
					CATAPULT_LOG(debug) << "exception thrown by get " << ex.what();
					isExceptionRaised = true;
				}

				isReadFinished = true;
			});
		});

		WAIT_FOR(isReadFinished);

		// Assert:
		EXPECT_FALSE(isExceptionRaised);
		TApiTraits::VerifyResult(result);
		handler(context.stats());
	}

	/// Asserts that the request initiated by \a requestInitator triggers an api exception using \a context
	/// and then calls \a handler.
	template<typename TTestContext, typename TRequestInitiator, typename THandler>
	void AssertInvalidRequestTriggersException(TTestContext&& context, TRequestInitiator requestInitator, THandler handler) {
		// Arrange:
		WaitForBoot(context);

		// Act:
		std::atomic_bool isReadFinished(false);
		ExternalSourceConnection connection;
		connection.apiCall([&](const auto& pRemoteChainApi) {
			requestInitator(*pRemoteChainApi).then([&, pRemoteChainApi](auto&& future) {
				// Act + Assert:
				EXPECT_THROW(future.get(), catapult_runtime_error);
				isReadFinished = true;
			});
		});

		WAIT_FOR(isReadFinished);

		// Assert:
		handler(context.stats());
	}

	// endregion

	// region height

	/// Gets the node height via \a connection.
	Height GetLocalNodeHeightViaApi(ExternalSourceConnection& connection);

	/// Waits for node height reported to \a connection to equal \a height.
	void WaitForLocalNodeHeight(ExternalSourceConnection& connection, Height height);

	// endregion
}}
