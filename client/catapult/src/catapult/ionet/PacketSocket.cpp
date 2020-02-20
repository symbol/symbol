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

#include "PacketSocket.h"
#include "BufferedPacketIo.h"
#include "Node.h"
#include "WorkingBuffer.h"
#include "catapult/thread/StrandOwnerLifetimeExtender.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/StackTimer.h"
#include <deque>
#include <memory>

namespace catapult { namespace ionet {

	namespace {
		// region AutoConsume

		class AutoConsume {
		public:
			explicit AutoConsume(PacketExtractor& packetExtractor) : m_packetExtractor(packetExtractor)
			{}

			~AutoConsume() {
				m_packetExtractor.consume();
			}

		private:
			PacketExtractor& m_packetExtractor;
		};

		// endregion

		// region error mapping

		SocketOperationCode mapReadErrorCodeToSocketOperationCode(const boost::system::error_code& ec) {
			if (!ec)
				return SocketOperationCode::Success;

			auto isEof = boost::asio::error::eof == ec;
			if (isEof)
				CATAPULT_LOG(info) << "eof reading from socket: " << ec.message();
			else
				CATAPULT_LOG(error) << "failed when reading from socket: " << ec.message();

			return isEof ? SocketOperationCode::Closed : SocketOperationCode::Read_Error;
		}

		SocketOperationCode mapWriteErrorCodeToSocketOperationCode(const boost::system::error_code& ec) {
			if (!ec)
				return SocketOperationCode::Success;

			CATAPULT_LOG(error) << "failed when writing to socket: " << ec.message();
			return SocketOperationCode::Write_Error;
		}

		// endregion

		// region BasicPacketSocket(Writer)

		template<typename TSocketCallbackWrapper>
		class BasicPacketSocketWriter {
		public:
			BasicPacketSocketWriter(ionet::socket& socket, TSocketCallbackWrapper& wrapper, size_t maxPacketDataSize)
					: m_socket(socket)
					, m_wrapper(wrapper)
					, m_maxPacketDataSize(maxPacketDataSize)
			{}

		public:
			void write(const PacketPayload& payload, const PacketSocket::WriteCallback& callback) {
				if (!IsPacketDataSizeValid(payload.header(), m_maxPacketDataSize)) {
					CATAPULT_LOG(warning) << "bypassing write of malformed " << payload.header();
					callback(SocketOperationCode::Malformed_Data);
					return;
				}

				auto pContext = std::make_shared<WriteContext>(payload, callback);
				boost::asio::async_write(m_socket, pContext->headerBuffer(), m_wrapper.wrap([this, pContext](const auto& ec, auto) {
					this->writeNext(ec, pContext);
				}));
			}

		private:
			struct WriteContext {
			public:
				WriteContext(const PacketPayload& payload, const PacketSocket::WriteCallback& callback)
						: m_payload(payload)
						, m_callback(callback)
						, m_nextBufferIndex(0)
				{}

			public:
				auto headerBuffer() const {
					const auto& header = m_payload.header();
					return boost::asio::buffer(reinterpret_cast<const uint8_t*>(&header), sizeof(header));
				}

				auto nextDataBuffer() {
					auto rawBuffer = m_payload.buffers()[m_nextBufferIndex++];
					return boost::asio::buffer(rawBuffer.pData, rawBuffer.Size);
				}

				bool tryComplete(const boost::system::error_code& ec) {
					auto lastCode = mapWriteErrorCodeToSocketOperationCode(ec);
					if (SocketOperationCode::Success != lastCode || m_nextBufferIndex >= m_payload.buffers().size()) {
						m_callback(lastCode);
						return true;
					}

					return false;
				}

			private:
				const PacketPayload m_payload;
				const PacketSocket::WriteCallback m_callback;
				size_t m_nextBufferIndex;
			};

			void writeNext(const boost::system::error_code& lastEc, const std::shared_ptr<WriteContext>& pContext) {
				if (pContext->tryComplete(lastEc))
					return;

				auto buffer = pContext->nextDataBuffer();
				boost::asio::async_write(m_socket, buffer, m_wrapper.wrap([this, pContext](const auto& ec, auto) {
					this->writeNext(ec, pContext);
				}));
			}

		private:
			socket& m_socket;
			TSocketCallbackWrapper& m_wrapper;
			size_t m_maxPacketDataSize;
		};

		// endregion

		// region BasicPacketSocket(Reader)

		template<typename TSocketCallbackWrapper>
		class BasicPacketSocketReader {
		public:
			BasicPacketSocketReader(ionet::socket& socket, TSocketCallbackWrapper& wrapper, WorkingBuffer& buffer)
					: m_socket(socket)
					, m_wrapper(wrapper)
					, m_buffer(buffer)
			{}

		public:
			void read(const PacketSocket::ReadCallback& callback, bool allowMultiple) {
				// try to extract a packet from the working buffer
				const Packet* pExtractedPacket = nullptr;
				auto packetExtractor = m_buffer.preparePacketExtractor();

				AutoConsume autoConsume(packetExtractor);
				auto extractResult = packetExtractor.tryExtractNextPacket(pExtractedPacket);

				switch (extractResult) {
				case PacketExtractResult::Success:
					do {
						callback(SocketOperationCode::Success, pExtractedPacket);
						if (!allowMultiple)
							return;

						extractResult = packetExtractor.tryExtractNextPacket(pExtractedPacket);
					} while (PacketExtractResult::Success == extractResult);
					return checkAndHandleError(extractResult, callback, allowMultiple);

				case PacketExtractResult::Insufficient_Data:
					break;

				default:
					return checkAndHandleError(extractResult, callback, allowMultiple);
				}

				// Read additional data from the socket and append it to the working buffer.
				// Note that readSome is only called when extractor returns Insufficient_Data, which also means no data was consumed
				// thus, the in-place read will have exclusive access to the working buffer and autoConsume's destruction will be a no-op.
				readSome(callback, allowMultiple);
			}

		private:
			struct SharedAppendContext {
			public:
				explicit SharedAppendContext(AppendContext&& context) : Context(std::move(context))
				{}

			public:
				AppendContext Context;
			};

			void readSome(const PacketSocket::ReadCallback& callback, bool allowMultiple) {
				auto pAppendContext = std::make_shared<SharedAppendContext>(m_buffer.prepareAppend());
				auto readHandler = [this, callback, allowMultiple, pAppendContext](const auto& ec, auto bytesReceived) {
					auto code = mapReadErrorCodeToSocketOperationCode(ec);
					if (SocketOperationCode::Success != code)
						return callback(code, nullptr);

					pAppendContext->Context.commit(bytesReceived);
					this->read(callback, allowMultiple);
				};

				m_socket.async_read_some(pAppendContext->Context.buffer(), m_wrapper.wrap(readHandler));
			}

			void checkAndHandleError(PacketExtractResult extractResult, const PacketSocket::ReadCallback& callback, bool allowMultiple) {
				// ignore non errors
				switch (extractResult) {
				case PacketExtractResult::Success:
					return;

				case PacketExtractResult::Insufficient_Data:
					// signal the completion of a multi-read operation
					if (allowMultiple)
						callback(SocketOperationCode::Insufficient_Data, nullptr);

					// this is not a termination condition for a single-read operation
					return;

				default:
					break;
				}

				// invoke the callback for errors
				CATAPULT_LOG(error) << "failed processing malformed packet: " << extractResult;
				callback(SocketOperationCode::Malformed_Data, nullptr);
			}

		private:
			socket& m_socket;
			TSocketCallbackWrapper& m_wrapper;
			WorkingBuffer& m_buffer;
		};

		// endregion

		// region BasicPacketSocket

		// implements packet based socket conventions with an implicit strand
		// \note user callbacks are executed in the context of the strand, so they are effectively serialized.
		template<typename TSocketCallbackWrapper>
		class BasicPacketSocket final
				: public BasicPacketSocketWriter<TSocketCallbackWrapper>
				, public BasicPacketSocketReader<TSocketCallbackWrapper> {
		public:
			BasicPacketSocket(boost::asio::io_context& ioContext, const PacketSocketOptions& options, TSocketCallbackWrapper& wrapper)
					: BasicPacketSocketWriter<TSocketCallbackWrapper>(m_socket, wrapper, options.MaxPacketDataSize)
					, BasicPacketSocketReader<TSocketCallbackWrapper>(m_socket, wrapper, m_buffer)
					, m_socket(ioContext)
					, m_buffer(options)
					, m_wrapper(wrapper)
			{}

		public:
			void stats(const PacketSocket::StatsCallback& callback) {
				PacketSocket::Stats stats;
				stats.IsOpen = m_socket.is_open();
				stats.NumUnprocessedBytes = m_buffer.size();
				callback(stats);
			}

			void waitForData(const PacketSocket::WaitForDataCallback& callback) {
				if (0 != m_buffer.size()) {
					callback();
					return;
				}

				m_socket.async_wait(socket::wait_read, m_wrapper.wrap([this, callback](const auto&) {
					if (!m_socket.is_open())
						return;

					// try to (non-blocking) read a single byte from the stream
					// this will skip any and all protocol-level data (e.g. ssl handshake)
					uint8_t peekByte;
					boost::system::error_code ignoredEc;
					auto numBytesRead = m_socket.read_some(boost::asio::buffer(&peekByte, 1), ignoredEc);

					// if a byte was successfully read, trigger the callback
					if (1 == numBytesRead) {
						m_buffer.append(peekByte);
						callback();
					}
				}));
			}

			void close() {
				boost::system::error_code ignored_ec;
				m_socket.shutdown(socket::shutdown_both, ignored_ec);
				m_socket.close(ignored_ec);
			}

		public:
			socket& impl() {
				return m_socket;
			}

			void setOptions() {
				m_socket.non_blocking(true);
			}

		private:
			socket m_socket;
			WorkingBuffer m_buffer;
			TSocketCallbackWrapper& m_wrapper;
		};

		// endregion

		// region StrandedPacketSocket

		class SocketIdentifier {
		public:
			explicit SocketIdentifier(uint64_t id)
					: m_id(id)
					, m_isClosed(false)
			{}

		public:
			bool fetchClose() {
				if (m_isClosed)
					return true;

				m_isClosed = true;
				return false;
			}

		public:
			friend std::ostream& operator<<(std::ostream& out, const SocketIdentifier& id) {
				out << "(" << utils::HexFormat(id.m_id) << ")";
				if (id.m_isClosed)
					out << ", " << id.m_timer.millis() << "ms elapsed";

				return out;
			}

		private:
			uint64_t m_id;
			bool m_isClosed;
			utils::StackTimer m_timer;
		};

		// implements PacketSocket using an explicit strand and ensures deterministic shutdown by using enable_shared_from_this
		class StrandedPacketSocket final
				: public PacketSocket
				, public std::enable_shared_from_this<StrandedPacketSocket> {
		private:
			using SocketType = BasicPacketSocket<StrandedPacketSocket>;

		public:
			StrandedPacketSocket(boost::asio::io_context& ioContext, const PacketSocketOptions& options)
					: m_strand(ioContext)
					, m_strandWrapper(m_strand)
					, m_socket(ioContext, options, *this)
					, m_id(s_idCounter.fetch_add(1))
			{}

			~StrandedPacketSocket() override {
				// all async operations posted on the strand must be completed by now because all operations
				// posted on the strand have been initiated by this object and captured this as a shared_ptr
				// (executing the destructor means they all must have been destroyed)

				// closing the socket is safe (this is the only thread left) and the strand can be destroyed
				// because it has been emptied
				if (m_id.fetchClose())
					return;

				CATAPULT_LOG(debug) << "socket close triggered by destruction " << m_id;
				m_socket.close();
			}

		public:
			void write(const PacketPayload& payload, const WriteCallback& callback) override {
				post([payload, callback](auto& socket) { socket.write(payload, callback); });
			}

			void read(const ReadCallback& callback) override {
				post([callback](auto& socket) { socket.read(callback, false); });
			}

			void readMultiple(const ReadCallback& callback) override {
				post([callback](auto& socket) { socket.read(callback, true); });
			}

			void stats(const StatsCallback& callback) override {
				post([callback](auto& socket) { socket.stats(callback); });
			}

			void waitForData(const WaitForDataCallback& callback) override {
				post([callback](auto& socket) { socket.waitForData(callback); });
			}

			void close() override {
				post([&id = m_id](auto& socket) {
					if (id.fetchClose())
						return;

					CATAPULT_LOG(debug) << "socket close triggered by owner " << id;
					socket.close();
				});
			}

			std::shared_ptr<PacketIo> buffered() override {
				return CreateBufferedPacketIo(shared_from_this(), m_strand);
			}

		public:
			socket& impl() {
				return m_socket.impl();
			}

			boost::asio::io_context::strand& strand() {
				return m_strand;
			}

			const SocketIdentifier& id() const {
				return m_id;
			}

			void setOptions() {
				// post is not required because call is made immediately after opening socket
				// so there is no opportunity for multithreaded access
				m_socket.setOptions();
			}

		public:
			template<typename THandler>
			auto wrap(THandler handler) {
				// when BasicPacketSocket calls wrap, the returned callback needs to extend the lifetime of this object
				return m_strandWrapper.wrap(shared_from_this(), handler);
			}

		private:
			template<typename THandler>
			void post(THandler handler) {
				// ensure all handlers extend the lifetime of this object and post to a strand
				return m_strandWrapper.post(shared_from_this(), [handler](const auto& pThis) {
					handler(pThis->m_socket);
				});
			}

		private:
			static std::atomic<uint64_t> s_idCounter;

		private:
			boost::asio::io_context::strand m_strand;
			thread::StrandOwnerLifetimeExtender<StrandedPacketSocket> m_strandWrapper;
			SocketType m_socket;
			SocketIdentifier m_id;
		};

		std::atomic<uint64_t> StrandedPacketSocket::s_idCounter(1);

		// endregion
	}

	// region Accept

	namespace {
		class AcceptHandler : public std::enable_shared_from_this<AcceptHandler> {
		public:
			AcceptHandler(
					boost::asio::io_context& ioContext,
					boost::asio::ip::tcp::acceptor& acceptor,
					const PacketSocketOptions& options,
					const ConfigureSocketCallback& configureSocket,
					const AcceptCallback& accept)
					: m_acceptor(acceptor)
					, m_configureSocket(configureSocket)
					, m_accept(accept)
					, m_pSocket(std::make_shared<StrandedPacketSocket>(ioContext, options))
			{}

		public:
			void start() {
				m_configureSocket(m_pSocket->impl());
				m_acceptor.async_accept(m_pSocket->impl(), [pThis = shared_from_this()](const auto& ec) {
					pThis->handleAccept(ec);
				});
			}

		private:
			void handleAccept(const boost::system::error_code& ec) {
				if (ec) {
					CATAPULT_LOG(warning) << "async_accept returned an error: " << ec;
					return m_accept(PacketSocketInfo());
				}

				// try to determine the remote endpoint (ignore errors if socket was immediately closed after accept)
				boost::system::error_code remoteEndpointEc;
				const auto& asioEndpoint = m_pSocket->impl().remote_endpoint(remoteEndpointEc);
				if (remoteEndpointEc) {
					CATAPULT_LOG(warning) << "unable to determine remote endpoint: " << remoteEndpointEc;
					return m_accept(PacketSocketInfo());
				}

				CATAPULT_LOG(debug) << "invoking user callback after successful async_accept " << m_pSocket->id();
				m_pSocket->setOptions();
				return m_accept(PacketSocketInfo(asioEndpoint.address().to_string(), m_pSocket));
			}

		private:
			boost::asio::ip::tcp::acceptor& m_acceptor;
			ConfigureSocketCallback m_configureSocket;
			AcceptCallback m_accept;
			std::shared_ptr<StrandedPacketSocket> m_pSocket;
		};
	}

	void Accept(
			boost::asio::io_context& ioContext,
			boost::asio::ip::tcp::acceptor& acceptor,
			const PacketSocketOptions& options,
			const ConfigureSocketCallback& configureSocket,
			const AcceptCallback& accept) {
		auto pHandler = std::make_shared<AcceptHandler>(ioContext, acceptor, options, configureSocket, accept);
		pHandler->start();
	}

	void Accept(
			boost::asio::io_context& ioContext,
			boost::asio::ip::tcp::acceptor& acceptor,
			const PacketSocketOptions& options,
			const AcceptCallback& accept) {
		Accept(ioContext, acceptor, options, [](const auto&) {}, accept);
	}

	// endregion

	// region Connect

	namespace {
		// basic connect handler implementation using an implicit strand
		template<typename TCallbackWrapper>
		class BasicConnectHandler final {
		private:
			using Resolver = boost::asio::ip::tcp::resolver;

		public:
			BasicConnectHandler(
					boost::asio::io_context& ioContext,
					const PacketSocketOptions& options,
					const NodeEndpoint& endpoint,
					const ConnectCallback& callback,
					TCallbackWrapper& wrapper)
					: m_callback(callback)
					, m_wrapper(wrapper)
					, m_pSocket(std::make_shared<StrandedPacketSocket>(ioContext, options))
					, m_resolver(ioContext)
					, m_host(endpoint.Host)
					, m_query(m_host, std::to_string(endpoint.Port))
					, m_isCancelled(false)
			{}

		public:
			void start() {
				m_resolver.async_resolve(m_query, m_wrapper.wrap([this](const auto& ec, auto iterator) {
					this->handleResolve(ec, iterator);
				}));
			}

			void cancel() {
				m_isCancelled = true;
				m_resolver.cancel();
				m_pSocket->close();
			}

		public:
			StrandedPacketSocket& impl() {
				return *m_pSocket;
			}

		private:
			void handleResolve(const boost::system::error_code& ec, const Resolver::iterator& iterator) {
				if (shouldAbort(ec, "resolving address"))
					return invokeCallback(ConnectResult::Resolve_Error);

				m_endpoint = iterator->endpoint();
				m_pSocket->impl().async_connect(m_endpoint, m_wrapper.wrap([this](const auto& connectEc) {
					this->handleConnect(connectEc);
				}));
			}

			void handleConnect(const boost::system::error_code& ec) {
				if (shouldAbort(ec, "connecting to"))
					return invokeCallback(ConnectResult::Connect_Error);

				CATAPULT_LOG(info) << "connected to " << m_host << " [" << m_endpoint << "] " << m_pSocket->id();
				return invokeCallback(ConnectResult::Connected);
			}

			bool shouldAbort(const boost::system::error_code& ec, const char* operation) {
				if (!ec && !m_isCancelled)
					return false;

				CATAPULT_LOG(error)
						<< "failed when " << operation << " '" << m_host << "': " << ec.message()
						<< " (cancelled? " << m_isCancelled << ")";
				return true;
			}

			void invokeCallback(ConnectResult result) {
				// if the cancelled flag is set, override the result
				auto callbackResult = m_isCancelled ? ConnectResult::Connect_Cancelled : result;
				if (ConnectResult::Connected == callbackResult) {
					m_pSocket->setOptions();
					m_callback(callbackResult, PacketSocketInfo(m_endpoint.address().to_string(), m_pSocket));
				} else {
					m_callback(callbackResult, PacketSocketInfo());
				}
			}

		private:
			ConnectCallback m_callback;
			TCallbackWrapper& m_wrapper;

			std::shared_ptr<StrandedPacketSocket> m_pSocket;
			Resolver m_resolver;
			std::string m_host;
			Resolver::query m_query;
			bool m_isCancelled;
			boost::asio::ip::tcp::endpoint m_endpoint;
		};

		// implements connect handler using an explicit strand and ensures deterministic shutdown by using enable_shared_from_this
		class StrandedConnectHandler : public std::enable_shared_from_this<StrandedConnectHandler> {
		public:
			StrandedConnectHandler(
					boost::asio::io_context& ioContext,
					const PacketSocketOptions& options,
					const NodeEndpoint& endpoint,
					const ConnectCallback& callback)
					: m_handler(ioContext, options, endpoint, callback, *this)
					, m_strandWrapper(m_handler.impl().strand()) // use the socket's strand
			{}

		public:
			void start() {
				post([](auto& handler) { handler.start(); });
			}

			void cancel() {
				post([](auto& handler) { handler.cancel(); });
			}

		public:
			template<typename THandler>
			auto wrap(THandler handler) {
				return m_strandWrapper.wrap(shared_from_this(), handler);
			}

		private:
			template<typename THandler>
			void post(THandler handler) {
				return m_strandWrapper.post(shared_from_this(), [handler](const auto& pThis) {
					handler(pThis->m_handler);
				});
			}

		private:
			BasicConnectHandler<StrandedConnectHandler> m_handler;
			thread::StrandOwnerLifetimeExtender<StrandedConnectHandler> m_strandWrapper;
		};
	}

	action Connect(
			boost::asio::io_context& ioContext,
			const PacketSocketOptions& options,
			const NodeEndpoint& endpoint,
			const ConnectCallback& callback) {
		auto pHandler = std::make_shared<StrandedConnectHandler>(ioContext, options, endpoint, callback);
		pHandler->start();
		return [pHandler] { pHandler->cancel(); };
	}

	// endregion
}}
