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

#include "ReadRateMonitorPacketIo.h"
#include "BatchPacketReader.h"
#include "PacketIo.h"

namespace catapult { namespace ionet {

	namespace {
		class ReadRateMonitorReadCallback {
		public:
			ReadRateMonitorReadCallback(const consumer<uint32_t>& readSizeConsumer, PacketIo::ReadCallback callback)
					: m_readSizeConsumer(readSizeConsumer)
					, m_callback(callback)
			{}

		public:
			void operator()(SocketOperationCode code, const Packet* pPacket) {
				m_callback(code, pPacket);

				if (pPacket)
					m_readSizeConsumer(pPacket->Size);
			}

		private:
			consumer<uint32_t> m_readSizeConsumer;
			PacketIo::ReadCallback m_callback;
		};

		class ReadRateMonitorPacketIo : public PacketIo {
		public:
			ReadRateMonitorPacketIo(const std::shared_ptr<PacketIo>& pIo, const consumer<uint32_t>& readSizeConsumer)
					: m_pIo(pIo)
					, m_readSizeConsumer(readSizeConsumer)
			{}

		public:
			void write(const PacketPayload& payload, const WriteCallback& callback) override {
				m_pIo->write(payload, callback);
			}

			void read(const ReadCallback& callback) override {
				auto rateMonitorCallback = ReadRateMonitorReadCallback(m_readSizeConsumer, callback);
				m_pIo->read(rateMonitorCallback);
			}

		private:
			std::shared_ptr<PacketIo> m_pIo;
			consumer<uint32_t> m_readSizeConsumer;
		};
	}

	std::shared_ptr<PacketIo> CreateReadRateMonitorPacketIo(
			const std::shared_ptr<PacketIo>& pIo,
			const consumer<uint32_t>& readSizeConsumer) {
		return std::make_shared<ReadRateMonitorPacketIo>(pIo, readSizeConsumer);
	}

	namespace {
		class ReadRateMonitorBatchPacketReader : public BatchPacketReader {
		public:
			ReadRateMonitorBatchPacketReader(const std::shared_ptr<BatchPacketReader>& pReader, const consumer<uint32_t>& readSizeConsumer)
					: m_pReader(pReader)
					, m_readSizeConsumer(readSizeConsumer)
			{}

		public:
			void readMultiple(const PacketIo::ReadCallback& callback) override {
				auto rateMonitorCallback = ReadRateMonitorReadCallback(m_readSizeConsumer, callback);
				m_pReader->readMultiple(rateMonitorCallback);
			}

		private:
			std::shared_ptr<BatchPacketReader> m_pReader;
			consumer<uint32_t> m_readSizeConsumer;
		};
	}

	std::shared_ptr<BatchPacketReader> CreateReadRateMonitorBatchPacketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const consumer<uint32_t>& readSizeConsumer) {
		return std::make_shared<ReadRateMonitorBatchPacketReader>(pReader, readSizeConsumer);
	}
}}
