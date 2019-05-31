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
#include "PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	template<typename TTraits>
	struct PushHandlerTests {
	public:
		class PushHandlerBuffer {
		public:
			PushHandlerBuffer(size_t count, bool validBuffer)
					: m_buffer(sizeof(ionet::Packet) + count * TTraits::Data_Size + TTraits::AdditionalPacketSize(count)) {
				test::FillWithRandomData(m_buffer);

				// set the packet at the start of the buffer
				auto& packet = reinterpret_cast<ionet::Packet&>(m_buffer[0]);
				packet.Size = static_cast<uint32_t>(m_buffer.size() - (validBuffer ? 0 : 1));
				packet.Type = TTraits::Packet_Type;

				TTraits::PreparePacket(m_buffer, count);

				if (!validBuffer)
					m_buffer.resize(m_buffer.size() - 1);
			}

			const ionet::Packet& packet() const {
				return reinterpret_cast<const ionet::Packet&>(m_buffer[0]);
			}

			ionet::ByteBuffer& buffer() {
				return m_buffer;
			}

		private:
			ionet::ByteBuffer m_buffer;
		};

		template<typename TRegisterHandler, typename TAction>
		static void RunPushTransactionsHandlerTest(TRegisterHandler registerHandler, const ionet::Packet& packet, TAction action) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto registry = TTraits::CreateRegistry();

			Key capturedSourcePublicKey;
			std::vector<size_t> counters;
			registerHandler(handlers, registry, [&capturedSourcePublicKey, &counters](const auto& range) {
				capturedSourcePublicKey = range.SourcePublicKey;
				counters.push_back(range.Range.size());
			});

			// Act:
			auto sourcePublicKey = test::GenerateRandomByteArray<Key>();
			ionet::ServerPacketHandlerContext context(sourcePublicKey, "");
			EXPECT_TRUE(handlers.process(packet, context));

			// Assert:
			action(counters);

			// - if the callback was called, context should have been forwarded along with the range
			if (!counters.empty())
				EXPECT_EQ(sourcePublicKey, capturedSourcePublicKey);
		}

		static void AssertMalformedPacketIsRejected() {
			// Arrange:
			PushHandlerBuffer buffer(3, false);

			// Act:
			RunPushTransactionsHandlerTest(TTraits::RegisterHandler, buffer.packet(), [](const auto& counters) {
				// Assert:
				EXPECT_TRUE(counters.empty());
			});
		}

		static void AssertValidPacketWithValidTransactionsIsAccepted() {
			// Arrange:
			PushHandlerBuffer buffer(3, true);

			// Act:
			RunPushTransactionsHandlerTest(TTraits::RegisterHandler, buffer.packet(), [](const auto& counters) {
				// Assert:
				ASSERT_EQ(1u, counters.size());
				EXPECT_EQ(3u, counters[0]);
			});
		}
	};

#define MAKE_PUSH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::PushHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_PUSH_HANDLER_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_PUSH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, MalformedPacketIsRejected) \
	MAKE_PUSH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithValidTransactionsIsAccepted)
}}
