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

#include "ClientSocket.h"
#include "SocketTestUtils.h"
#include "catapult/thread/StrandOwnerLifetimeExtender.h"
#include "catapult/exceptions.h"
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>

namespace catapult { namespace test {

	namespace {
		std::string ToMessage(const boost::system::error_code& ec) {
			return ec ? "- with code " + ec.message() : "- success!";
		}

		template<typename TPromise>
		void SetPromiseException(TPromise& promise, const boost::system::error_code& ec) {
			promise.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
		}

		class SocketGuard : public std::enable_shared_from_this<SocketGuard>{
		public:
			explicit SocketGuard(boost::asio::io_context& ioContext)
					: m_strand(ioContext)
					, m_strandWrapper(m_strand)
					, m_socket(ioContext, CreatePacketSocketSslOptions().ContextSupplier())
					, m_sentinelByte(0) {
				m_isClosed.clear();
			}

		public:
			auto& get() {
				return m_socket;
			}

		public:
			void close() {
				closeOnce([this]() {
					CATAPULT_LOG(debug) << "closing client socket via close";
					post([this](auto& socket) {
						socket.async_shutdown(wrap([](const auto& ec) {
							CATAPULT_LOG(debug) << "async_shutdown completed " << ec.message();
						}));
					});

					post([this](auto& socket) {
						auto asioBuffer = boost::asio::buffer(&m_sentinelByte, 1);
						boost::asio::async_write(socket, asioBuffer, wrap([this](const auto& ec, auto) {
							CATAPULT_LOG(debug) << "async_write completed " << ec.message();
							closeInternal();
						}));
					});
				});
			}

			void abort() {
				closeOnce([this]() {
					CATAPULT_LOG(debug) << "closing client socket via abort";
					closeInternal();
				});
			}

		private:
			void closeInternal() {
				boost::system::error_code ignoredEc;
				m_socket.lowest_layer().close(ignoredEc);
			}

			template<typename THandler>
			void closeOnce(THandler handler) {
				if (m_isClosed.test_and_set())
					return;

				handler();
			}

		private:
			template<typename THandler>
			auto wrap(THandler handler) {
				// when BasicPacketSocket calls wrap, the returned callback needs to extend the lifetime of this object
				return m_strandWrapper.wrap(shared_from_this(), handler);
			}

			template<typename THandler>
			void post(THandler handler) {
				// ensure all handlers extend the lifetime of this object and post to a strand
				return m_strandWrapper.post(shared_from_this(), [handler](const auto& pThis) {
					handler(pThis->m_socket);
				});
			}

		private:
			boost::asio::io_context::strand m_strand;
			thread::StrandOwnerLifetimeExtender<SocketGuard> m_strandWrapper;
			ionet::Socket m_socket;
			uint8_t m_sentinelByte;
			std::atomic_flag m_isClosed;
		};

		class DefaultClientSocket : public ClientSocket, public std::enable_shared_from_this<DefaultClientSocket> {
		public:
			explicit DefaultClientSocket(boost::asio::io_context& ioContext)
					: m_pSocketGuard(std::make_shared<SocketGuard>(ioContext))
					, m_socket(m_pSocketGuard->get())
					, m_timer(ioContext)
			{}

			~DefaultClientSocket() override {
				shutdown();
			}

		public:
			thread::future<ClientSocket*> connect(ConnectOptions options) override {
				return connect(GetLocalHostPort(), options);
			}

			thread::future<ClientSocket*> connect(unsigned short port, ConnectOptions options) override {
				auto pPromise = std::make_shared<thread::promise<ClientSocket*>>();
				auto future = pPromise->get_future();

				// connect to the local host
				auto endpoint = CreateLocalHostEndpoint(port);
				CATAPULT_LOG(debug) << "attempting client socket connection to " << endpoint;
				m_socket.lowest_layer().async_connect(endpoint, [pThis = shared_from_this(), options, pPromise](const auto& ec) {
					CATAPULT_LOG(debug) << "client socket connected " << ToMessage(ec);
					if (ec)
						return SetPromiseException(*pPromise, ec);

					if (ConnectOptions::Abort == options) {
						CATAPULT_LOG(debug) << "aborting client socket connection";
						pThis->m_socket.lowest_layer().set_option(boost::asio::socket_base::linger(true, 0));
						pThis->m_socket.lowest_layer().close();
						CATAPULT_LOG(debug) << "aborted client socket connection";
					}

					if (ConnectOptions::Skip_Handshake == options) {
						CATAPULT_LOG(debug) << "skipping client socket handshake";
						pPromise->set_value(pThis.get());
						return;
					}

					pThis->m_socket.async_handshake(ionet::Socket::client, [pThis, pPromise](const auto& handshakeEc) {
						if (handshakeEc)
							return SetPromiseException(*pPromise, handshakeEc);

						pPromise->set_value(pThis.get());
					});
				});

				return future;
			}

		public:
			thread::future<size_t> read(ionet::ByteBuffer& receiveBuffer) override {
				auto pPromise = std::make_shared<thread::promise<size_t>>();
				auto future = pPromise->get_future();

				// perform the read
				auto asioBuffer = boost::asio::buffer(receiveBuffer);
				boost::asio::async_read(m_socket, asioBuffer, [pThis = shared_from_this(), pPromise](const auto& ec, auto numBytes) {
					CATAPULT_LOG(debug) << "client socket read " << numBytes << " bytes " << ToMessage(ec);
					if (ec && boost::asio::error::eof != ec)
						return SetPromiseException(*pPromise, ec);

					pPromise->set_value(std::move(numBytes));
				});

				return future;
			}

		private:
			struct WriteContext {
				WriteContext(const std::vector<ionet::ByteBuffer>& sendBuffers, size_t delayMillis)
						: SendBuffers(sendBuffers) // make a copy of the send buffers
						, DelayMillis(delayMillis)
						, NextId(0)
						, NumTotalBytesWritten(0)
				{}

				const std::vector<ionet::ByteBuffer> SendBuffers;
				size_t DelayMillis;
				size_t NextId;
				size_t NumTotalBytesWritten;
			};

		public:
			thread::future<size_t> write(const ionet::ByteBuffer& sendBuffer) override {
				return write({ sendBuffer }, 0);
			}

			thread::future<size_t> write(const std::vector<ionet::ByteBuffer>& sendBuffers, size_t delayMillis) override {
				auto pPromise = std::make_shared<thread::promise<size_t>>();
				auto future = pPromise->get_future();

				auto pWriteContext = std::make_shared<WriteContext>(sendBuffers, delayMillis);
				startWrite(pWriteContext, pPromise);
				return future;
			}

		private:
			void startWrite(const std::shared_ptr<WriteContext>& pWriteContext, const std::shared_ptr<thread::promise<size_t>>& pPromise) {
				// stop - all buffers have been written
				auto& nextId = pWriteContext->NextId;
				const auto& sendBuffers = pWriteContext->SendBuffers;
				if (nextId >= sendBuffers.size()) {
					CATAPULT_LOG(debug) << "done writing " << sendBuffers.size() << " buffers";
					pPromise->set_value(std::move(pWriteContext->NumTotalBytesWritten));
					return;
				}

				// perform the write
				CATAPULT_LOG(debug) << "writing buffer " << (nextId + 1) << " / " << sendBuffers.size();
				boost::asio::async_write(
						m_socket,
						boost::asio::buffer(sendBuffers[nextId++]),
						[pThis = shared_from_this(), pWriteContext, pPromise](const auto& ec, auto numBytes) {
					CATAPULT_LOG(debug) << "client socket wrote " << numBytes << " bytes " << ToMessage(ec);
					pWriteContext->NumTotalBytesWritten += numBytes;

					if (ec)
						return SetPromiseException(*pPromise, ec);

					pThis->delay(
							[pThis, pWriteContext, pPromise]() {
								pThis->startWrite(pWriteContext, pPromise);
							},
							pWriteContext->DelayMillis);
				});
			}

		public:
			void delay(const action& continuation, size_t delayMillis) override {
				// if there is no delay, just invoke the continuation
				if (0 == delayMillis) {
					continuation();
					return;
				}

				// otherwise, set a timer
				CATAPULT_LOG(debug) << "delaying for " << delayMillis << "ms";
				m_timer.expires_from_now(std::chrono::milliseconds(delayMillis));
				m_timer.async_wait([continuation](const auto& ec) {
					CATAPULT_LOG(debug) << "resuming " << ToMessage(ec);
					continuation();
				});
			}

			void shutdown() override {
				m_pSocketGuard->close();
			}

			void abort() override {
				m_pSocketGuard->abort();
			}

		private:
			std::shared_ptr<SocketGuard> m_pSocketGuard;
			ionet::Socket& m_socket;
			boost::asio::steady_timer m_timer;
		};
	}

	std::shared_ptr<ClientSocket> CreateClientSocket(boost::asio::io_context& ioContext) {
		return std::make_shared<DefaultClientSocket>(ioContext);
	}

	std::shared_ptr<ClientSocket> AddClientConnectionTask(boost::asio::io_context& ioContext) {
		auto pClientSocket = CreateClientSocket(ioContext);
		pClientSocket->connect();
		return pClientSocket;
	}

	std::shared_ptr<ClientSocket> AddClientReadBufferTask(boost::asio::io_context& ioContext, ionet::ByteBuffer& receiveBuffer) {
		auto pClientSocket = CreateClientSocket(ioContext);
		pClientSocket->connect().then([&receiveBuffer](auto&& socketFuture) {
			socketFuture.get()->read(receiveBuffer);
		});
		return pClientSocket;
	}

	std::shared_ptr<ClientSocket> AddClientWriteBuffersTask(
			boost::asio::io_context& ioContext,
			const std::vector<ionet::ByteBuffer>& sendBuffers) {
		auto pClientSocket = CreateClientSocket(ioContext);
		pClientSocket->connect().then([sendBuffers](auto&& socketFuture) {
			socketFuture.get()->write(sendBuffers);
		});
		return pClientSocket;
	}
}}
