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

#include "catapult/handlers/HandlerFactory.h"
#include "catapult/handlers/BasicProducer.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace handlers {

#define TEST_CLASS HandlerFactoryTests

	namespace {
		constexpr auto Valid_Packet_Type = static_cast<ionet::PacketType>(17);
		constexpr auto Valid_Payload_Size = sizeof(uint64_t);
	}

	// region SimpleEntityTraits and Producer

	namespace {
#pragma pack(push, 1)

		struct SimpleEntity {
			uint32_t Size;
			uint32_t Seed;
			uint16_t Value;
			uint16_t Reserved1;
		};

#pragma pack(pop)

		struct SimpleEntityTraits {
			using RequestStructureType = uint64_t;

			static constexpr auto Packet_Type = Valid_Packet_Type;

			struct Producer {
			public:
				Producer(uint16_t seed, uint16_t count)
						: m_seed(seed)
						, m_count(count)
						, m_index(0)
				{}

			public:
				std::shared_ptr<SimpleEntity> operator()() {
					if (m_index >= m_count)
						return nullptr;

					auto pEntity = std::make_shared<SimpleEntity>();
					pEntity->Size = sizeof(SimpleEntity);
					pEntity->Seed = m_seed;
					pEntity->Value = m_index++;
					return pEntity;
				}

			private:
				uint16_t m_seed;
				uint16_t m_count;
				uint16_t m_index;
			};
		};

		struct SimpleEntityAsValuesTraits : public SimpleEntityTraits {
			static constexpr auto Should_Append_As_Values = true;
		};
	}

	// endregion

	// region utils

	namespace {
		template<typename TRegisterHandler>
		void AssertRequestPacketIsRejected(
				uint32_t payloadSize,
				ionet::PacketType type,
				uint32_t adjustmentSize,
				TRegisterHandler registerHandler) {
			// Arrange:
			auto pPacket = test::CreateRandomPacket(payloadSize, type);
			pPacket->Size -= adjustmentSize;

			size_t numHandlerCalls = 0;
			ionet::ServerPacketHandlers handlers;
			registerHandler(handlers, [&numHandlerCalls]() {
				++numHandlerCalls;
				return SimpleEntityTraits::Producer(1, 0);
			});

			// Sanity: the handler was registered
			EXPECT_EQ(1u, handlers.size());
			EXPECT_TRUE(handlers.canProcess(Valid_Packet_Type));

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			handlers.process(*pPacket, handlerContext);

			// Assert: the handler was not called
			EXPECT_EQ(0u, numHandlerCalls);
			EXPECT_FALSE(handlerContext.hasResponse());
		}
	}

	// endregion

	// region RegisterZero

	namespace {
		void AssertZeroRequestPacketIsRejected(uint32_t payloadSize, ionet::PacketType type, uint32_t adjustmentSize = 0) {
			// Assert:
			AssertRequestPacketIsRejected(payloadSize, type, adjustmentSize, [](auto& handlers, auto action) {
				BatchHandlerFactory<SimpleEntityTraits>::RegisterZero(handlers, action);
			});
		}
	}

	TEST(TEST_CLASS, RegisterZero_RequestPacketTooSmallIsRejected) {
		AssertZeroRequestPacketIsRejected(0, Valid_Packet_Type, 1);
	}

	TEST(TEST_CLASS, RegisterZero_RequestPacketTooLargeIsRejected) {
		AssertZeroRequestPacketIsRejected(1, Valid_Packet_Type);
	}

	TEST(TEST_CLASS, RegisterZero_RequestPacketWithWrongTypeIsRejected) {
		AssertZeroRequestPacketIsRejected(0, ionet::PacketType::Pull_Transactions);
	}

	namespace {
		template<typename TEntityTraits, typename TAction>
		void RunZeroValidRequestTest(ionet::ServerPacketHandlers&& handlers, TAction action) {
			// Arrange:
			auto pPacket = test::CreateRandomPacket(0, Valid_Packet_Type);

			size_t numHandlerCalls = 0;
			BatchHandlerFactory<TEntityTraits>::RegisterZero(handlers, [&numHandlerCalls]() {
				++numHandlerCalls;
				return SimpleEntityTraits::Producer(12, 3);
			});

			// Sanity: the handler was registered
			EXPECT_EQ(1u, handlers.size());
			EXPECT_TRUE(handlers.canProcess(Valid_Packet_Type));

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			handlers.process(*pPacket, handlerContext);

			// Assert: the handler was called
			EXPECT_EQ(1u, numHandlerCalls);
			action(handlerContext);
		}
	}

	TEST(TEST_CLASS, RegisterZero_EmptyRequestPacketIsAccepted) {
		// Arrange:
		RunZeroValidRequestTest<SimpleEntityTraits>(ionet::ServerPacketHandlers(), [](const auto& handlerContext) {
			// Assert:
			test::AssertPacketHeader(handlerContext.response(), sizeof(ionet::Packet) + 3 * sizeof(SimpleEntity), Valid_Packet_Type);

			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(3u, buffers.size());
			for (auto i = 0u; i < buffers.size(); ++i) {
				auto message = "buffer at " + std::to_string(i);
				ASSERT_EQ(sizeof(SimpleEntity), buffers[i].Size) << message;

				const auto& entity = reinterpret_cast<const SimpleEntity&>(*buffers[i].pData);
				ASSERT_EQ(sizeof(SimpleEntity), entity.Size);
				EXPECT_EQ(12u, entity.Seed);
				EXPECT_EQ(i, entity.Value);
			}
		});
	}

	TEST(TEST_CLASS, RegisterZero_EmptyRequestPacketIsAccepted_AsValues) {
		// Arrange:
		RunZeroValidRequestTest<SimpleEntityAsValuesTraits>(ionet::ServerPacketHandlers(), [](const auto& handlerContext) {
			// Assert:
			test::AssertPacketHeader(handlerContext.response(), sizeof(ionet::Packet) + 3 * sizeof(SimpleEntity), Valid_Packet_Type);

			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(1u, buffers.size());
			ASSERT_EQ(3u * sizeof(SimpleEntity), buffers[0].Size);

			const auto* pSimpleEntity = reinterpret_cast<const SimpleEntity*>(buffers[0].pData);
			for (auto i = 0u; i < 3; ++i, ++pSimpleEntity) {
				ASSERT_EQ(sizeof(SimpleEntity), pSimpleEntity->Size);
				EXPECT_EQ(12u, pSimpleEntity->Seed);
				EXPECT_EQ(i, pSimpleEntity->Value);
			}
		});
	}

	TEST(TEST_CLASS, RegisterZero_MaxPacketDataSizeIsRespected) {
		// Arrange:
		uint32_t maxPacketDataSize = sizeof(SimpleEntity) * 3 / 2;
		RunZeroValidRequestTest<SimpleEntityTraits>(ionet::ServerPacketHandlers(maxPacketDataSize), [](const auto& handlerContext) {
			// Assert: response was aborted because it was too large
			EXPECT_TRUE(handlerContext.hasResponse());
			test::AssertPacketPayloadUnset(handlerContext.response());
		});
	}

	TEST(TEST_CLASS, RegisterZero_MaxPacketDataSizeIsRespected_AsValues) {
		// Arrange:
		uint32_t maxPacketDataSize = sizeof(SimpleEntity) * 3 / 2;
		RunZeroValidRequestTest<SimpleEntityAsValuesTraits>(ionet::ServerPacketHandlers(maxPacketDataSize), [](
				const auto& handlerContext) {
			// Assert: response was aborted because it was too large
			EXPECT_TRUE(handlerContext.hasResponse());
			test::AssertPacketPayloadUnset(handlerContext.response());
		});
	}

	// endregion

	// region BatchHandlerFactory

	namespace {
		void AssertOneRequestPacketIsRejected(uint32_t payloadSize, ionet::PacketType type, uint32_t adjustmentSize = 0) {
			// Assert:
			AssertRequestPacketIsRejected(payloadSize, type, adjustmentSize, [](auto& handlers, auto action) {
				BatchHandlerFactory<SimpleEntityTraits>::RegisterOne(handlers, [action](const auto&) {
					return action();
				});
			});
		}
	}

	TEST(TEST_CLASS, RegisterOne_RequestPacketWithNoPayloadIsRejected) {
		// Assert: no payload
		AssertOneRequestPacketIsRejected(0, Valid_Packet_Type);
	}

	TEST(TEST_CLASS, RegisterOne_RequestPacketTooSmallIsRejected) {
		AssertOneRequestPacketIsRejected(Valid_Payload_Size, Valid_Packet_Type, 1);
	}

	TEST(TEST_CLASS, RegisterOne_RequestPacketWithInvalidPayloadIsRejected) {
		// Assert: payload size is not divisible by Valid_Payload_Size
		AssertOneRequestPacketIsRejected(Valid_Payload_Size + 5, Valid_Packet_Type);
	}

	TEST(TEST_CLASS, RegisterOne_RequestPacketWithWrongTypeIsRejected) {
		// Assert: wrong packet type
		AssertOneRequestPacketIsRejected(Valid_Payload_Size, ionet::PacketType::Pull_Transactions);
	}

	namespace {
		template<typename TEntityTraits, typename TAction>
		void RunOneValidRequestTest(ionet::ServerPacketHandlers&& handlers, uint16_t numProducedValues, TAction action) {
			// Arrange: encode the seed and count in the request packet to ensure the values are passed correctly
			auto pPacket = test::CreateRandomPacket(2 * Valid_Payload_Size, Valid_Packet_Type);
			auto* pRequestData = reinterpret_cast<uint64_t*>(pPacket->Data());
			pRequestData[0] = 12;
			pRequestData[1] = numProducedValues;

			size_t numHandlerCalls = 0;
			BatchHandlerFactory<TEntityTraits>::RegisterOne(handlers, [&numHandlerCalls](const auto& values) {
				++numHandlerCalls;

				auto seed = static_cast<uint16_t>(*values.cbegin());
				auto count = static_cast<uint16_t>(*++values.cbegin());
				return SimpleEntityTraits::Producer(seed, count);
			});

			// Sanity: the handler was registered
			EXPECT_EQ(1u, handlers.size());
			EXPECT_TRUE(handlers.canProcess(Valid_Packet_Type));

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			handlers.process(*pPacket, handlerContext);

			// Assert: the handler was called
			EXPECT_EQ(1u, numHandlerCalls);
			action(handlerContext);
		}
	}

	TEST(TEST_CLASS, RegisterOne_ValidRequestPacketCanYieldEmptyResponse) {
		// Arrange:
		RunOneValidRequestTest<SimpleEntityTraits>(ionet::ServerPacketHandlers(), 0, [](const auto& handlerContext) {
			// Assert: response packet is header only
			test::AssertPacketHeader(handlerContext.response(), sizeof(ionet::Packet), Valid_Packet_Type);
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, RegisterOne_ValidRequestPacketCanYieldEmptyResponse_AsValues) {
		// Arrange:
		RunOneValidRequestTest<SimpleEntityAsValuesTraits>(ionet::ServerPacketHandlers(), 0, [](const auto& handlerContext) {
			// Assert: response packet is header only
			test::AssertPacketHeader(handlerContext.response(), sizeof(ionet::Packet), Valid_Packet_Type);
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, RegisterOne_ValidRequestPacketCanYieldResponse) {
		// Arrange:
		RunOneValidRequestTest<SimpleEntityTraits>(ionet::ServerPacketHandlers(), 3, [](const auto& handlerContext) {
			// Assert:
			test::AssertPacketHeader(handlerContext.response(), sizeof(ionet::Packet) + 3 * sizeof(SimpleEntity), Valid_Packet_Type);

			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(3u, buffers.size());
			for (auto i = 0u; i < buffers.size(); ++i) {
				auto message = "buffer at " + std::to_string(i);
				ASSERT_EQ(sizeof(SimpleEntity), buffers[i].Size) << message;

				const auto& entity = reinterpret_cast<const SimpleEntity&>(*buffers[i].pData);
				ASSERT_EQ(sizeof(SimpleEntity), entity.Size);
				EXPECT_EQ(12u, entity.Seed);
				EXPECT_EQ(i, entity.Value);
			}
		});
	}

	TEST(TEST_CLASS, RegisterOne_ValidRequestPacketCanYieldResponse_AsValues) {
		// Arrange:
		RunOneValidRequestTest<SimpleEntityAsValuesTraits>(ionet::ServerPacketHandlers(), 3, [](const auto& handlerContext) {
			// Assert:
			test::AssertPacketHeader(handlerContext.response(), sizeof(ionet::Packet) + 3 * sizeof(SimpleEntity), Valid_Packet_Type);

			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(1u, buffers.size());
			ASSERT_EQ(3u * sizeof(SimpleEntity), buffers[0].Size);

			const auto* pSimpleEntity = reinterpret_cast<const SimpleEntity*>(buffers[0].pData);
			for (auto i = 0u; i < 3; ++i, ++pSimpleEntity) {
				ASSERT_EQ(sizeof(SimpleEntity), pSimpleEntity->Size);
				EXPECT_EQ(12u, pSimpleEntity->Seed);
				EXPECT_EQ(i, pSimpleEntity->Value);
			}
		});
	}

	TEST(TEST_CLASS, RegisterOne_MaxPacketDataSizeIsRespected) {
		// Arrange:
		auto handlers = ionet::ServerPacketHandlers(sizeof(SimpleEntity) * 3 / 2);
		RunOneValidRequestTest<SimpleEntityTraits>(std::move(handlers), 3, [](const auto& handlerContext) {
			// Assert: response was aborted because it was too large
			EXPECT_TRUE(handlerContext.hasResponse());
			test::AssertPacketPayloadUnset(handlerContext.response());
		});
	}

	TEST(TEST_CLASS, RegisterOne_MaxPacketDataSizeIsRespected_AsValues) {
		// Arrange:
		auto handlers = ionet::ServerPacketHandlers(sizeof(SimpleEntity) * 3 / 2);
		RunOneValidRequestTest<SimpleEntityAsValuesTraits>(std::move(handlers), 3, [](const auto& handlerContext) {
			// Assert: response was aborted because it was too large
			EXPECT_TRUE(handlerContext.hasResponse());
			test::AssertPacketPayloadUnset(handlerContext.response());
		});
	}

	// endregion
}}
