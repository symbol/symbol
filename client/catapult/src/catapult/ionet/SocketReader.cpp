/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "SocketReader.h"
#include "PacketSocket.h"

namespace catapult { namespace ionet {

	namespace {
		namespace operation_state {
			constexpr auto Unknown = 0u;
			constexpr auto Read_Done = 1u;
			constexpr auto Error_Raised = 2u;
		}

		// encapsulates a packet read and write operation
		class PacketReadWriteOperation : public std::enable_shared_from_this<PacketReadWriteOperation> {
		public:
			PacketReadWriteOperation(
					const std::shared_ptr<BatchPacketReader>& pReader,
					const std::shared_ptr<PacketIo>& pWriter,
					const ServerPacketHandlers& handlers,
					const model::NodeIdentity& identity,
					const SocketReader::ReadCallback& callback)
					: m_pReader(pReader)
					, m_pWriter(pWriter)
					, m_handlers(handlers)
					, m_identity(identity)
					, m_callback(callback)
					, m_numOutstandingOperations(0)
					, m_state(operation_state::Unknown)
			{}

		public:
			bool isComplete() const {
				// only indicate completion if a termination condition (i.e. known state) has been reached
				return 0 == m_numOutstandingOperations && operation_state::Unknown != m_state;
			}

			void start() {
				// read one or more packets; this is safe because
				// (1) the writes are buffered
				// (2) the read and write callbacks are serialized
				m_pReader->readMultiple([pThis = shared_from_this()](auto code, const auto* pPacket) {
					pThis->handleReadCallback(code, pPacket);
				});
			}

		private:
			void handleReadCallback(SocketOperationCode code, const Packet* pPacket) {
				++m_numOutstandingOperations;
				if (SocketOperationCode::Success != code)
					return invokeCallback(code);

				ServerPacketHandlerContext handlerContext(m_identity.PublicKey, m_identity.Host);
				if (!m_handlers.process(*pPacket, handlerContext)) {
					CATAPULT_LOG(warning) << m_identity << " ignoring unknown packet of type " << pPacket->Type;
					return invokeCallback(SocketOperationCode::Malformed_Data);
				}

				// if the handlers didn't prepare a response, return and don't write anything
				if (!handlerContext.hasResponse())
					return invokeCallback(SocketOperationCode::Success);

				write(handlerContext);
			}

			void write(const ServerPacketHandlerContext& handlerContext) {
				m_pWriter->write(handlerContext.response(), [pThis = shared_from_this()](auto code) {
					pThis->handleWriteCallback(code);
				});
			}

			void handleWriteCallback(SocketOperationCode code) {
				invokeCallback(code);
			}

			void invokeCallback(SocketOperationCode code) {
				--m_numOutstandingOperations;
				updateState(code);

				// save Insufficient_Data errors for last
				if (SocketOperationCode::Insufficient_Data != code)
					m_callback(code);

				// trigger insufficient data when all sub operations have completed and none have failed
				if (0 == m_numOutstandingOperations && operation_state::Read_Done == m_state)
					return m_callback(SocketOperationCode::Insufficient_Data);
			}

			void updateState(SocketOperationCode code) {
				switch (code) {
				case SocketOperationCode::Success:
					break;

				case SocketOperationCode::Insufficient_Data:
					m_state |= operation_state::Read_Done;
					break;

				default:
					m_state |= operation_state::Error_Raised;
					break;
				}
			}

		private:
			std::shared_ptr<BatchPacketReader> m_pReader;
			std::shared_ptr<PacketIo> m_pWriter;
			const ServerPacketHandlers& m_handlers;
			const model::NodeIdentity& m_identity;
			SocketReader::ReadCallback m_callback;

			// these need to be atomic because `isComplete` reads them outside of underlying socket's strand
			std::atomic<size_t> m_numOutstandingOperations;
			std::atomic<size_t> m_state;
		};

		class DefaultSocketReader : public SocketReader {
		public:
			DefaultSocketReader(
					const std::shared_ptr<BatchPacketReader>& pReader,
					const std::shared_ptr<PacketIo>& pWriter,
					const ServerPacketHandlers& handlers,
					const model::NodeIdentity& identity)
					: m_pReader(pReader)
					, m_pWriter(pWriter)
					, m_handlers(handlers)
					, m_identity(identity)
			{}

		public:
			void read(const ReadCallback& callback) override {
				if (!canRead())
					CATAPULT_THROW_RUNTIME_ERROR("simultaneous reads are not supported");

				auto pOperation = makeOperation(callback);
				pOperation->start();

				m_pOperation = pOperation;
			}

		private:
			bool canRead() const {
				auto pOperation = m_pOperation.lock();
				return !pOperation || pOperation->isComplete();
			}

			std::shared_ptr<PacketReadWriteOperation> makeOperation(const ReadCallback& callback) {
				return std::make_shared<PacketReadWriteOperation>(m_pReader, m_pWriter, m_handlers, m_identity, callback);
			}

		private:
			std::shared_ptr<BatchPacketReader> m_pReader;
			std::shared_ptr<PacketIo> m_pWriter;
			std::weak_ptr<PacketReadWriteOperation> m_pOperation;
			const ServerPacketHandlers& m_handlers; // handlers are unique per process and externally owned (by PacketReaders)
			model::NodeIdentity m_identity; // identity is copied because it is unique per socket
		};
	}

	std::unique_ptr<SocketReader> CreateSocketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const std::shared_ptr<PacketIo>& pWriter,
			const ServerPacketHandlers& handlers,
			const model::NodeIdentity& identity) {
		return std::make_unique<DefaultSocketReader>(pReader, pWriter, handlers, identity);
	}
}}
