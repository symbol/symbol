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

#include "finalization/src/handlers/FinalizationHandlers.h"
#include "tests/test/core/PushHandlerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS FinalizationHandlersTests

	namespace {
		void SetMessageAt(ionet::ByteBuffer& buffer, size_t offset, size_t size) {
			auto messageSize = static_cast<uint32_t>(size);
			auto remainingBufferSize = buffer.size() - offset;

			if (messageSize > remainingBufferSize)
				throw std::runtime_error("cannot fit the data in provided buffer");

			auto& message = reinterpret_cast<model::FinalizationMessage&>(buffer[offset]);
			message.Size = messageSize;
			message.HashesCount = static_cast<uint32_t>((size - sizeof(model::FinalizationMessage)) / Hash256::Size);
		}

		struct PushMessagesTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Push_Finalization_Messages;
			static constexpr auto Data_Size = sizeof(model::FinalizationMessage);

			static constexpr size_t AdditionalPacketSize(size_t numMessages) {
				return (numMessages * (numMessages + 1) / 2) * Hash256::Size;
			}

			static void PreparePacket(ionet::ByteBuffer& buffer, size_t count) {
				auto currentOffset = sizeof(ionet::Packet);
				for (auto i = 0u; i < count; ++i) {
					auto size = Data_Size + (i + 1) * Hash256::Size;
					SetMessageAt(buffer, currentOffset, size);
					currentOffset += size;
				}
			}

			static auto CreateRegistry() {
				// note that int is used as a placeholder transaction registy
				// because a real one is not needed by RegisterPushMessagesHandler
				return 7;
			}

			static auto RegisterHandler(ionet::ServerPacketHandlers& handlers, int, const MessageRangeHandler& rangeHandler) {
				return RegisterPushMessagesHandler(handlers, rangeHandler);
			}
		};
	}

	DEFINE_PUSH_HANDLER_TESTS(TEST_CLASS, PushMessages)
}}
