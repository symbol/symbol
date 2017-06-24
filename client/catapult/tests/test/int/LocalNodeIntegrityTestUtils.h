#pragma once
#include "catapult/api/RemoteChainApi.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/net/ServerConnector.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"

namespace catapult { namespace test {

	/// Represents an external source connection.
	class ExternalSourceConnection {
	public:
		/// Creates an external source connection
		ExternalSourceConnection()
				: m_pPool(CreateStartedIoServiceThreadPool(1))
				, m_clientKeyPair(crypto::KeyPair::FromPrivate(GenerateRandomPrivateKey()))
				, m_pConnector(net::CreateServerConnector(m_pPool, m_clientKeyPair, net::ConnectionSettings()))
				, m_localNode(CreateLocalHostNode(LoadServerKeyPair().publicKey(), Local_Host_Port))
		{}

	public:
		std::shared_ptr<ionet::PacketIo> io() const {
			return m_pIo;
		}

	public:
		/// Connects to the local node and calls \a onConnect on completion.
		void connect(const std::function<void (const std::shared_ptr<ionet::PacketSocket>&)>& onConnect) {
			m_pConnector->connect(m_localNode, [&pIo = m_pIo, onConnect](auto connectResult, const auto& pPacketSocket) -> void {
				// save pIo in a member to tie the lifetime of the connection to the lifetime of the owning ExternalSourceConnection
				pIo = pPacketSocket;
				if (net::PeerConnectResult::Accepted == connectResult)
					onConnect(pPacketSocket);
			});
		}

		/// Connects to the local node and calls \a onConnect on completion.
		void apiCall(const std::function<void (const api::RemoteChainApi&)>& onConnect) {
			connect([onConnect](const auto& pPacketSocket) {
				auto pApi = api::CreateRemoteChainApi(pPacketSocket, CreateTransactionRegistry());
				onConnect(*pApi);
			});
		}

	private:
		static std::shared_ptr<model::TransactionRegistry> CreateTransactionRegistry();

	private:
		std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
		crypto::KeyPair m_clientKeyPair;
		std::shared_ptr<net::ServerConnector> m_pConnector;
		ionet::Node m_localNode;

		std::shared_ptr<ionet::PacketIo> m_pIo;
	};

	/// Waits for \a context to be booted.
	template<typename TTestContext>
	void WaitForBoot(const TTestContext& context) {
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveReaders);
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveWriters);
	}

	// region push

	/// Pushes \a pEntity as a packet of \a type to \a connection.
	template<typename TEntity>
	std::shared_ptr<ionet::PacketIo> PushEntity(
			ExternalSourceConnection& connection,
			ionet::PacketType type,
			const std::shared_ptr<TEntity>& pEntity) {
		CATAPULT_LOG(debug) << " >>>> starting push";
		std::atomic_bool isWriteFinished(false);
		connection.connect([&](const auto& pPacketSocket) -> void {
			auto pPacket = ionet::PacketPayload::FromEntity(type, pEntity);

			CATAPULT_LOG(debug) << "writing entity";
			pPacketSocket->write(pPacket, [&isWriteFinished](auto code) {
				CATAPULT_LOG(debug) << "write result: " << code;
				isWriteFinished = true;
			});
		});

		WAIT_FOR(isWriteFinished);
		CATAPULT_LOG(debug) << " <<<< push finished";
		return connection.io();
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
		connection.apiCall([&](const auto& remoteApi) -> void {
			TApiTraits::InitiateValidRequest(remoteApi).then([&](auto&& future) {
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
		connection.apiCall([&](const auto& remoteApi) -> void {
			requestInitator(remoteApi).then([&](auto&& future) {
				// Assert:
				EXPECT_THROW(future.get(), catapult_runtime_error);
				isReadFinished = true;
			});
		});

		WAIT_FOR(isReadFinished);

		// Assert:
		handler(context.stats());
	}

	// endregion
}}
