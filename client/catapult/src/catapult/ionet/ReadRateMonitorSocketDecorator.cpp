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

#include "ReadRateMonitorSocketDecorator.h"
#include "PacketSocketDecorator.h"
#include "RateMonitor.h"
#include "ReadRateMonitorPacketIo.h"

namespace catapult { namespace ionet {

	namespace {
		class ReadRateMonitorWrapFactory {
		public:
			ReadRateMonitorWrapFactory(
					const RateMonitorSettings& settings,
					const supplier<Timestamp>& timeSupplier,
					const action& rateExceededHandler)
					: m_pMonitor(std::make_shared<RateMonitor>(settings, timeSupplier, rateExceededHandler))
			{}

		public:
			auto wrapIo(const std::shared_ptr<PacketIo>& pIo) const {
				return CreateReadRateMonitorPacketIo(pIo, readSizeConsumer());
			}

			auto wrapReader(const std::shared_ptr<BatchPacketReader>& pReader) const {
				return CreateReadRateMonitorBatchPacketReader(pReader, readSizeConsumer());
			}

		private:
			consumer<uint32_t> readSizeConsumer() const {
				return [pMonitor = m_pMonitor](auto size) {
					pMonitor->accept(size);
				};
			}

		private:
			std::shared_ptr<RateMonitor> m_pMonitor;
		};
	}

	std::shared_ptr<PacketSocket> AddReadRateMonitor(
			const std::shared_ptr<PacketSocket>& pSocket,
			const RateMonitorSettings& settings,
			const supplier<Timestamp>& timeSupplier,
			const action& rateExceededHandler) {
		if (0 == settings.NumBuckets)
			return pSocket;

		ReadRateMonitorWrapFactory wrapFactory(settings, timeSupplier, rateExceededHandler);
		return std::make_shared<PacketSocketDecorator<ReadRateMonitorWrapFactory>>(pSocket, wrapFactory);
	}
}}
