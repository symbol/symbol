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

#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace ionet {

#define TEST_CLASS BufferedPacketIoTests

	namespace {
		std::shared_ptr<PacketIo> Transform(const std::shared_ptr<PacketSocket>& pSocket) {
			return pSocket->buffered();
		}
	}

	TEST(TEST_CLASS, WriteCanWriteMultipleConsecutivePayloads) {
		test::AssertWriteCanWriteMultipleConsecutivePayloads(Transform);
	}

	TEST(TEST_CLASS, WriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving) {
		test::AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(Transform);
	}

	TEST(TEST_CLASS, ReadCanReadMultipleConsecutivePayloads) {
		test::AssertReadCanReadMultipleConsecutivePayloads(Transform);
	}

	TEST(TEST_CLASS, ReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving) {
		test::AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(Transform);
	}
}}
