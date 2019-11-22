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
#include "MockPacketIo.h"
#include "catapult/ionet/PacketSocket.h"

namespace catapult { namespace mocks {

	/// Mock packet socket for use in decorator tests.
	class MockPacketSocket : public ionet::PacketSocket, public MockPacketIo {
	public:
		/// Creates a socket.
		MockPacketSocket()
				: m_numStatsCalls(0)
				, m_numCloseCalls(0)
				, m_numBufferedCalls(0)
				, m_pBufferedIo(std::make_shared<MockPacketIo>())
		{}

	public:
		/// Number of times stats was called.
		size_t numStatsCalls() const {
			return m_numStatsCalls;
		}

		/// Number of times close was called.
		size_t numCloseCalls() const {
			return m_numCloseCalls;
		}

		/// Number of times buffered was called.
		size_t numBufferedCalls() const {
			return m_numBufferedCalls;
		}

	public:
		/// Underlying mock packet io.
		auto mockBufferedIo() {
			return m_pBufferedIo;
		}

		/// Configures buffered io to support a roundtrip operation.
		void enableBufferedIoRoundtrip() {
			// allow a write / read roundtrip so the buffered roundtrip test will pass
			m_pBufferedIo->queueWrite(ionet::SocketOperationCode::Success);
			m_pBufferedIo->queueRead(ionet::SocketOperationCode::Success, [](const auto* pWrittenPacket) {
				auto pWrittenPacketCopy = utils::MakeSharedWithSize<ionet::Packet>(pWrittenPacket->Size);
				std::memcpy(static_cast<void*>(pWrittenPacketCopy.get()), pWrittenPacket, pWrittenPacket->Size);
				return pWrittenPacketCopy;
			});
		}

	public:
		void write(const ionet::PacketPayload& payload, const WriteCallback& callback) override {
			MockPacketIo::write(payload, callback);
		}

		void read(const ReadCallback& callback) override {
			MockPacketIo::read(callback);
		}

		void readMultiple(const ReadCallback& callback) override {
			MockPacketIo::readMultiple(callback);
		}

	public:
		void stats(const StatsCallback& callback) override {
			callback({ true, ++m_numStatsCalls });
		}

		void waitForData(const WaitForDataCallback& callback) override {
			callback();
		}

		void close() override{
			++m_numCloseCalls;
		}

		std::shared_ptr<PacketIo> buffered() override {
			++m_numBufferedCalls;
			return m_pBufferedIo;
		}

	private:
		size_t m_numStatsCalls;
		size_t m_numCloseCalls;
		size_t m_numBufferedCalls;

		std::shared_ptr<MockPacketIo> m_pBufferedIo;
	};
}}
