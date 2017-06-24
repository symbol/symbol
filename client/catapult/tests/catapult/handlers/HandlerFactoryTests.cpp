#include "catapult/handlers/HandlerFactory.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {
		constexpr auto Valid_Packet_Type = static_cast<ionet::PacketType>(17);
		constexpr auto Valid_Payload_Size = sizeof(uint64_t);

#pragma pack(push, 1)

		struct SimpleEntity {
			uint16_t Alpha;
			uint32_t Beta;
			uint16_t Gamma;
		};

#pragma pack(pop)

		struct SimpleEntitiesTraits {
			// notice that the supplier supplies results as SupplierResultsType
			// notice that SimpleEntity is intentionally the same size as uint64_t for test purposes
			using EntityType = SimpleEntity;
			using SupplierResultsType = std::vector<uint64_t>;

			static constexpr auto Packet_Type = Valid_Packet_Type;

			static auto ToPayload(const SupplierResultsType& results) {
				auto payloadSize = utils::checked_cast<size_t, uint32_t>(results.size() * Valid_Payload_Size);
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pPacket->Type = Packet_Type;
				std::memcpy(pPacket->Data(), results.data(), payloadSize);
				return pPacket;
			}
		};

		class HandlerContext {
		public:
			bool hasPayload() const {
				return !!m_pPayload;
			}

			const ionet::PacketPayload& payload() const {
				return *m_pPayload;
			}

		public:
			void response(ionet::PacketPayload&& payload) {
				m_pPayload = std::make_unique<ionet::PacketPayload>(payload);
			}

		private:
			// stored as unique_ptr to allow optionality
			std::unique_ptr<ionet::PacketPayload> m_pPayload;
		};

		void AssertPacketIsRejected(const ionet::Packet& packet) {
			// Arrange:
			size_t numHandlerCalls = 0;
			auto handler = BatchHandlerFactory<SimpleEntitiesTraits>::Create([&numHandlerCalls](const auto&) {
				++numHandlerCalls;
				return std::vector<uint64_t>();
			});

			HandlerContext context;

			// Act:
			handler(packet, context);

			// Assert: the handler was not called
			EXPECT_EQ(0u, numHandlerCalls);
			EXPECT_FALSE(context.hasPayload());
		}

		void AssertPacketIsRejected(uint32_t payloadSize, ionet::PacketType type) {
			// Arrange:
			auto pPacket = test::CreateRandomPacket(payloadSize, type);

			// Assert:
			AssertPacketIsRejected(*pPacket);
		}
	}

	// region failure

	TEST(HandlerFactoryTests, TooSmallPacketIsRejected) {
		// Arrange:
		ionet::Packet packet;
		packet.Size = sizeof(ionet::PacketHeader) - 1;
		packet.Type = Valid_Packet_Type;

		// Assert:
		AssertPacketIsRejected(packet);
	}

	TEST(HandlerFactoryTests, PacketWithWrongTypeIsRejected) {
		// Assert: wrong packet type
		AssertPacketIsRejected(Valid_Payload_Size, ionet::PacketType::Pull_Transactions);
	}

	TEST(HandlerFactoryTests, PacketWithInvalidPayloadIsRejected) {
		// Assert: payload size is not divisible by Valid_Payload_Size
		AssertPacketIsRejected(Valid_Payload_Size + 5, Valid_Packet_Type);
	}

	TEST(HandlerFactoryTests, PacketWithNoPayloadIsRejected) {
		// Assert: no payload
		AssertPacketIsRejected(0, Valid_Packet_Type);
	}

	// endregion

	// region success

	TEST(HandlerFactoryTests, ValidPacketWithNonEmptyPayloadIsAccepted) {
		// Arrange:
		size_t numHandlerCalls = 0;
		auto handler = BatchHandlerFactory<SimpleEntitiesTraits>::Create([&numHandlerCalls](const auto&) {
			++numHandlerCalls;
			return std::vector<uint64_t>();
		});

		auto pPacket = test::CreateRandomPacket(3 * Valid_Payload_Size, Valid_Packet_Type);
		HandlerContext context;

		// Act:
		handler(*pPacket, context);

		// Assert: the handler was called
		EXPECT_EQ(1u, numHandlerCalls);
		ASSERT_TRUE(context.hasPayload());
		test::AssertPacketHeader(context.payload(), sizeof(ionet::Packet), Valid_Packet_Type);
		EXPECT_TRUE(context.payload().buffers().empty());
	}

	namespace {
		void AssertResponseIsSetIfPacketIsValid(uint32_t numRequestEntities, uint32_t numResponseEntities) {
			// Arrange:
			size_t numHandlerCalls = 0;
			uint32_t numRequestBytes = numRequestEntities * sizeof(uint64_t);
			uint32_t numResponseBytes = numResponseEntities * sizeof(uint64_t);

			std::vector<uint64_t> requestEntities;
			std::vector<uint64_t> responseEntities(numResponseEntities);
			auto handler = BatchHandlerFactory<SimpleEntitiesTraits>::Create([&](const auto& range) {
				++numHandlerCalls;

				// - copy the request (cast the SimpleEntity values to uint64_t)
				for (const auto& entity : range)
					requestEntities.push_back(reinterpret_cast<const uint64_t&>(entity));

				// - generate a random response
				test::FillWithRandomData({ reinterpret_cast<uint8_t*>(responseEntities.data()), numResponseBytes });
				return responseEntities;
			});

			auto pRequestPacket = test::CreateRandomPacket(numRequestBytes, Valid_Packet_Type);
			HandlerContext context;

			// Act:
			handler(*pRequestPacket, context);

			// Assert: the requested entities were passed to the results supplier
			EXPECT_EQ(numRequestEntities, requestEntities.size());
			EXPECT_TRUE(0 == std::memcmp(pRequestPacket->Data(), requestEntities.data(), numRequestBytes));

			// - the handler was called and has the correct header
			EXPECT_EQ(1u, numHandlerCalls);
			ASSERT_TRUE(context.hasPayload());
			test::AssertPacketHeader(context.payload(), sizeof(ionet::PacketHeader) + numResponseBytes, Valid_Packet_Type);

			// - the entities returned by the results supplier were copied into the response packet as a single buffer
			ASSERT_EQ(1u, context.payload().buffers().size());
			const auto& buffer = context.payload().buffers()[0];
			EXPECT_EQ(numResponseBytes, buffer.Size);
			EXPECT_TRUE(0 == std::memcmp(responseEntities.data(), buffer.pData, numResponseBytes));
		}
	}

	TEST(HandlerFactoryTests, ResponseIsSetIfPacketIsValid) {
		// Assert:
		AssertResponseIsSetIfPacketIsValid(3, 1);
		AssertResponseIsSetIfPacketIsValid(5, 3);
		AssertResponseIsSetIfPacketIsValid(7, 10);
	}

	// endregion
}}
