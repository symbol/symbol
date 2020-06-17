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

#include "partialtransaction/src/handlers/CosignatureHandlers.h"
#include "tests/test/core/PushHandlerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS CosignatureHandlersTests

	namespace {
		struct PushCosignaturesTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Push_Detached_Cosignatures;
			static constexpr auto Data_Size = sizeof(model::DetachedCosignature);

			static constexpr size_t AdditionalPacketSize(size_t) {
				return 0u;
			}

			static void PreparePacket(ionet::ByteBuffer&, size_t)
			{}

			static auto CreateRegistry() {
				// note that int is used as a placeholder transaction registy
				// because a real one is not needed by RegisterPushCosignaturesHandler
				return 7;
			}

			static auto RegisterHandler(ionet::ServerPacketHandlers& handlers, int, const CosignatureRangeHandler& rangeHandler) {
				return RegisterPushCosignaturesHandler(handlers, rangeHandler);
			}
		};
	}

	DEFINE_PUSH_HANDLER_TESTS(TEST_CLASS, PushCosignatures)
}}
