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
#include "catapult/exceptions.h"
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

		struct DefaultClientSocket : public ClientSocket, public std::enable_shared_from_this<DefaultClientSocket> {
		public:
			explicit DefaultClientSocket(boost::asio::io_context& ioContext)
					: m_socket(ioContext)
					, m_timer(ioContext)
			{}

			~DefaultClientSocket() override {
				CATAPULT_LOG(debug) << "closing client socket";
				boost::system::error_code ec;
				m_socket.close(ec);
				CATAPULT_LOG(debug) << "client socket closed: " << ec.message();
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
				m_socket.async_connect(endpoint, [pThis = shared_from_this(), options, pPromise](const auto& ec) {
					CATAPULT_LOG(debug) << "client socket connected " << ToMessage(ec);
					if (ec)
						return SetPromiseException(*pPromise, ec);

					if (ConnectOptions::Abort == options) {
						CATAPULT_LOG(debug) << "aborting client socket connection";
						pThis->m_socket.set_option(boost::asio::socket_base::linger(true, 0));
						pThis->m_socket.close();
						CATAPULT_LOG(debug) << "aborted client socket connection";
					}

					pPromise->set_value(pThis.get());
				});

				return future;
			}

		public:
			thread::future<size_t> read(ionet::ByteBuffer& receiveBuffer) override {
				auto pPromise = std::make_shared<thread::promise<size_t>>();
				auto future = pPromise->get_future();

				// perform the read
				boost::asio::async_read(
						m_socket,
						boost::asio::buffer(receiveBuffer),
						[pThis = shared_from_this(), pPromise](const auto& ec, auto numBytes) {
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
				CATAPULT_LOG(debug) << "shutting down client socket";
				m_socket.shutdown(ionet::socket::shutdown_both);
				m_socket.close();
			}

		private:
			ionet::socket m_socket;
			boost::asio::steady_timer m_timer;
		};
	}

	std::shared_ptr<ClientSocket> CreateClientSocket(boost::asio::io_context& ioContext) {
		return std::make_shared<DefaultClientSocket>(ioContext);
	}

	void AddClientConnectionTask(boost::asio::io_context& ioContext) {
		CreateClientSocket(ioContext)->connect();
	}

	void AddClientReadBufferTask(boost::asio::io_context& ioContext, ionet::ByteBuffer& receiveBuffer) {
		CreateClientSocket(ioContext)->connect().then([&receiveBuffer](auto&& socketFuture) {
			socketFuture.get()->read(receiveBuffer);
		});
	}

	void AddClientWriteBuffersTask(boost::asio::io_context& ioContext, const std::vector<ionet::ByteBuffer>& sendBuffers) {
		CreateClientSocket(ioContext)->connect().then([sendBuffers](auto&& socketFuture) {
			socketFuture.get()->write(sendBuffers);
		});
	}
}}
