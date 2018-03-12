#include "BufferedPacketIo.h"
#include "PacketIo.h"
#include "catapult/utils/Logging.h"
#include <deque>

namespace catapult { namespace ionet {

	namespace {
		class WriteRequest {
		public:
			explicit WriteRequest(PacketIo& io, const PacketPayload& payload)
					: m_io(io)
					, m_payload(payload)
			{}

		public:
			template<typename TCallback>
			void invoke(TCallback callback) {
				m_io.write(m_payload, callback);
			}

		private:
			PacketIo& m_io;
			PacketPayload m_payload;
		};

		class ReadRequest {
		public:
			explicit ReadRequest(PacketIo& io) : m_io(io)
			{}

		public:
			template<typename TCallback>
			void invoke(TCallback callback) {
				m_io.read(callback);
			}

		private:
			PacketIo& m_io;
		};

		/// Simple queue implementation.
		template<typename TRequest, typename TCallback, typename TCallbackWrapper>
		class RequestQueue {
		public:
			explicit RequestQueue(TCallbackWrapper& wrapper) : m_wrapper(wrapper)
			{}

		public:
			void push(const TRequest& request, const TCallback& callback) {
				auto hasPendingWork = !m_requests.empty();
				m_requests.emplace_back(request, callback);

				if (hasPendingWork) {
					CATAPULT_LOG(trace) << "queuing work because in progress operation detected";
					return;
				}

				next();
			}

		private:
			void next() {
				// note that it's very important to not call pop_front here - the request should only be popped
				// after the callback is invoked (and the operation is complete)
				auto& request = m_requests.front();
				request.first.invoke(m_wrapper.wrap(WrappedWithRequests<TCallback>(request.second, *this)));
			}

			template<typename THandler>
			struct WrappedWithRequests {
				WrappedWithRequests(THandler handler, RequestQueue& queue)
						: m_handler(std::move(handler))
						, m_queue(queue)
				{}

				template<typename... TArgs>
				void operator()(TArgs ...args) {
					// pop the current request (the operation has completed)
					m_queue.m_requests.pop_front();

					// execute the user handler
					m_handler(std::forward<TArgs>(args)...);

					// if requests are pending, start the next one
					if (!m_queue.m_requests.empty())
						m_queue.next();
				}

			private:
				THandler m_handler;
				RequestQueue& m_queue;
			};

		private:
			TCallbackWrapper& m_wrapper;
			std::deque<std::pair<TRequest, TCallback>> m_requests;
		};

		/// Protects RequestQueue via a strand.
		template<typename TRequest, typename TCallback>
		class QueuedOperation {
		public:
			explicit QueuedOperation(boost::asio::strand& strand)
					: m_strand(strand)
					, m_requests(m_strand)
			{}

		public:
			void push(const TRequest& request, const TCallback& callback) {
				m_strand.post([this, request, callback] {
					m_requests.push(request, callback);
				});
			}

		private:
			boost::asio::strand& m_strand;
			RequestQueue<TRequest, TCallback, boost::asio::strand> m_requests;
		};

		using QueuedWriteOperation = QueuedOperation<WriteRequest, PacketIo::WriteCallback>;
		using QueuedReadOperation = QueuedOperation<ReadRequest, PacketIo::ReadCallback>;

		class DefaultBufferedPacketIo
				: public PacketIo
				, public std::enable_shared_from_this<DefaultBufferedPacketIo> {
		public:
			DefaultBufferedPacketIo(const std::shared_ptr<PacketIo>& pIo, boost::asio::strand& strand)
					: m_pIo(pIo)
					, m_strand(strand)
					, m_pWriteOperation(std::make_unique<QueuedWriteOperation>(m_strand))
					, m_pReadOperation(std::make_unique<QueuedReadOperation>(m_strand))
			{}

		public:
			void write(const PacketPayload& payload, const WriteCallback& callback) override {
				auto request = WriteRequest(*m_pIo, payload);
				m_pWriteOperation->push(request, [pThis = shared_from_this(), callback](auto code) {
					callback(code);
				});
			}

			void read(const ReadCallback& callback) override {
				auto request = ReadRequest(*m_pIo);
				m_pReadOperation->push(request, [pThis = shared_from_this(), callback](auto code, const auto* pPacket) {
					callback(code, pPacket);
				});
			}

		private:
			std::shared_ptr<PacketIo> m_pIo;
			boost::asio::strand& m_strand;
			std::unique_ptr<QueuedWriteOperation> m_pWriteOperation;
			std::unique_ptr<QueuedReadOperation> m_pReadOperation;
		};
	}

	std::shared_ptr<PacketIo> CreateBufferedPacketIo(const std::shared_ptr<PacketIo>& pIo, boost::asio::strand& strand) {
		return std::make_shared<DefaultBufferedPacketIo>(pIo, strand);
	}
}}
