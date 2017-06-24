#pragma once
#include "catapult/net/AsyncTcpServer.h"

namespace catapult { namespace mocks {

	/// A mock AsyncTcpServerAcceptContext that is a pair of a service and a socket with no side effects.
	class MockAsyncTcpServerAcceptContext : public net::AsyncTcpServerAcceptContext {
	public:
		/// Creates a mock accept context around \a service and \a pSocket.
		MockAsyncTcpServerAcceptContext(
				boost::asio::io_service& service,
				const std::shared_ptr<ionet::PacketSocket>& pSocket)
				: m_service(service)
				, m_pSocket(pSocket)
				{}

	public:
		boost::asio::io_service& service() override {
			return m_service;
		}

		std::shared_ptr<ionet::PacketSocket> socket() override {
			return m_pSocket;
		}

	private:
		boost::asio::io_service& m_service;
		std::shared_ptr<ionet::PacketSocket> m_pSocket;
	};
}}
