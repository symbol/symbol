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
#include "catapult/ionet/BatchPacketReader.h"
#include "catapult/ionet/PacketIo.h"
#include "tests/test/core/mocks/MockPacketIo.h"

namespace catapult { namespace test {

	/// Parameters passed to ionet::PacketIo::ReadCallback.
	struct PacketIoReadCallbackParams {
		/// \c true if the packet is valid.
		bool IsPacketValid;

		/// Read operation code.
		ionet::SocketOperationCode ReadCode;

		/// Copy of packet bytes.
		std::vector<uint8_t> ReadPacketBytes;
	};

	/// Creates a PacketIo read callback that captures the last parameters in \a capture.
	ionet::PacketIo::ReadCallback CreateReadCaptureCallback(PacketIoReadCallbackParams& capture);

	/// Creates a PacketIo read callback that captures all parameters in \a captures.
	ionet::PacketIo::ReadCallback CreateReadCaptureCallback(std::vector<PacketIoReadCallbackParams>& captures);

	/// Asserts that packets can be written to and read from \a io using \a mockIo to capture writes.
	void AssertCanRoundtripPackets(mocks::MockPacketIo& mockIo, ionet::PacketIo& io);

	/// Asserts that packets can be written to \a io and read from \a reader using \a mockIo to capture writes.
	void AssertCanRoundtripPackets(mocks::MockPacketIo& mockIo, ionet::PacketIo& io, ionet::BatchPacketReader& reader);
}}
