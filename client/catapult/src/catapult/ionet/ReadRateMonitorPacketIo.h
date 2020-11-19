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

#pragma once
#include "catapult/functions.h"
#include "catapult/types.h"
#include <memory>

namespace catapult {
	namespace ionet {
		class BatchPacketReader;
		class PacketIo;
	}
}

namespace catapult { namespace ionet {

	/// Adds read rate monitoring to all packets read from \a pIo by passing sizes of all read packets to \a readSizeConsumer.
	std::shared_ptr<PacketIo> CreateReadRateMonitorPacketIo(
			const std::shared_ptr<PacketIo>& pIo,
			const consumer<uint32_t>& readSizeConsumer);

	/// Adds read rate monitoring to all packets read from \a pReader by passing sizes of all read packets to \a readSizeConsumer.
	std::shared_ptr<BatchPacketReader> CreateReadRateMonitorBatchPacketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const consumer<uint32_t>& readSizeConsumer);
}}
