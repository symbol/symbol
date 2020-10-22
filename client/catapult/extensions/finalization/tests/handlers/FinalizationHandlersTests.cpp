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

#include "finalization/src/handlers/FinalizationHandlers.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PushHandlerTestUtils.h"
#include "tests/test/plugins/PullHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS FinalizationHandlersTests

	// region PushMessagesHandler

	namespace {
		constexpr auto Message_Size = model::FinalizationMessage::MinSize();

		void SetMessageAt(ionet::ByteBuffer& buffer, size_t offset, size_t size) {
			auto messageSize = static_cast<uint32_t>(size);
			auto remainingBufferSize = buffer.size() - offset;

			if (messageSize > remainingBufferSize)
				throw std::runtime_error("cannot fit the data in provided buffer");

			auto& message = reinterpret_cast<model::FinalizationMessage&>(buffer[offset]);
			message.Size = messageSize;
			message.SignatureScheme = 1;
			message.Data().HashesCount = static_cast<uint32_t>((size - Message_Size) / Hash256::Size);
		}

		struct PushMessagesTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Push_Finalization_Messages;
			static constexpr auto Data_Size = Message_Size;

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

	// endregion

	// region PullMessagesHandler - basic edge case tests

	namespace {
		struct PullMessagesTraits {
			static constexpr auto Data_Header_Size = sizeof(model::FinalizationRoundRange);
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Finalization_Messages;
			static constexpr auto Valid_Request_Payload_Size = SizeOf32<utils::ShortHash>();

			using ResponseType = std::vector<std::shared_ptr<const model::FinalizationMessage>>;
			using RetrieverParamType = utils::ShortHashesSet;

			using MessagesRetrieverAdapter = std::function<ResponseType (const utils::ShortHashesSet&)>;
			static auto RegisterHandler(ionet::ServerPacketHandlers& handlers, const MessagesRetrieverAdapter& messagesRetriever) {
				handlers::RegisterPullMessagesHandler(handlers, [messagesRetriever](auto, const auto& knownShortHashes) {
					return messagesRetriever(knownShortHashes);
				});
			}
		};

		DEFINE_PULL_HANDLER_EDGE_CASE_TESTS(TEST_CLASS, PullMessages)
	}

	// endregion

	// region PullMessagesHandler - request + response tests

	namespace {
		struct PullMessagesRequestResponseTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Finalization_Messages;
			static constexpr auto RegisterHandler = handlers::RegisterPullMessagesHandler;

			using FilterType = model::FinalizationRoundRange;

			class PullResponseContext {
			public:
				explicit PullResponseContext(size_t numResponseMessages) {
					for (uint16_t i = 0u; i < numResponseMessages; ++i)
						m_messages.push_back(test::CreateMessage(Height(100 + i), i));
				}

			public:
				const auto& response() const {
					return m_messages;
				}

				auto responseSize() const {
					return test::TotalSize(m_messages);
				}

				void assertPayload(const ionet::PacketPayload& payload) {
					ASSERT_EQ(m_messages.size(), payload.buffers().size());

					auto i = 0u;
					for (const auto& pExpectedMessage : m_messages) {
						const auto& message = reinterpret_cast<const model::FinalizationMessage&>(*payload.buffers()[i].pData);
						ASSERT_EQ(pExpectedMessage->Size, message.Size) << "message at " << i;
						EXPECT_EQ_MEMORY(pExpectedMessage.get(), &message, pExpectedMessage->Size);
						++i;
					}
				}

			private:
				std::vector<std::shared_ptr<const model::FinalizationMessage>> m_messages;
			};
		};
	}

	DEFINE_PULL_HANDLER_REQUEST_RESPONSE_TESTS(
			TEST_CLASS,
			PullMessages,
			test::PullEntitiesHandlerAssertAdapter<PullMessagesRequestResponseTraits>::AssertFunc)

	// endregion
}}
