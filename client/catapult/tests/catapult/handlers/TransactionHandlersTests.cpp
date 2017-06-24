#include "catapult/handlers/TransactionHandlers.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	// region PushTransactionsHandler

	namespace {
		class PushTransactionsBuffer {
		public:
			static constexpr auto Transaction_Size = sizeof(mocks::MockTransaction);

			PushTransactionsBuffer(size_t count, bool validBuffer)
					: m_buffer(sizeof(ionet::Packet) + count * Transaction_Size + count * (count + 1) / 2) {
				test::FillWithRandomData(m_buffer);

				// set the packet at the start of the buffer
				auto& packet = reinterpret_cast<ionet::Packet&>(*m_buffer.data());
				packet.Size = static_cast<uint32_t>(m_buffer.size() - (validBuffer ? 0 : 1));
				packet.Type = ionet::PacketType::Push_Transactions;

				auto currentOffset = sizeof(ionet::Packet);
				for (auto i = 0u; i < count; ++i) {
					auto size = Transaction_Size + i + 1;
					test::SetTransactionAt(m_buffer, currentOffset, size);
					currentOffset += size;
				}

				if (!validBuffer)
					m_buffer.resize(m_buffer.size() - 1);
			}

			const ionet::Packet& packet() const {
				return reinterpret_cast<const ionet::Packet&>(*m_buffer.data());
			}

		private:
			ionet::ByteBuffer m_buffer;
		};

		namespace {
			template<typename TAction>
			void RunPushTransactionsHandlerTest(const PushTransactionsBuffer& buffer, TAction action) {
				// Arrange:
				ionet::ServerPacketHandlers handlers;
				auto pRegistry = mocks::CreateDefaultTransactionRegistry();
				std::vector<size_t> counters;
				RegisterPushTransactionsHandler(
						handlers,
						*pRegistry,
						[&counters](const auto& range) { counters.push_back(range.size()); });

				// Act:
				ionet::ServerPacketHandlerContext context;
				EXPECT_TRUE(handlers.process(buffer.packet(), context));

				// Assert:
				action(counters);
			}
		}
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_MalformedPacketIsRejected) {
		// Arrange:
		RunPushTransactionsHandlerTest(PushTransactionsBuffer(7, false), [](const auto& counters) {
			// Assert:
			EXPECT_TRUE(counters.empty());
		});
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_ValidPacketIsAccepted) {
		// Arrange:
		RunPushTransactionsHandlerTest(PushTransactionsBuffer(7, true), [](const auto& counters) {
			// Assert:
			ASSERT_EQ(1u, counters.size());
			EXPECT_EQ(7u, counters[0]);
		});
	}

	// endregion

	// region PullTransactionsHandler

	namespace {
		std::shared_ptr<ionet::Packet> CreatePacket(uint32_t payloadSize) {
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
			pPacket->Type = ionet::PacketType::Pull_Transactions;

			// fill payload with random short hashes
			if (0 < payloadSize)
				test::FillWithRandomData({ pPacket->Data(), payloadSize });

			return pPacket;
		}

		void RegisterHandler(ionet::ServerPacketHandlers& handlers, size_t& counter) {
			RegisterPullTransactionsHandler(handlers, [&](const auto&) {
				++counter;
				return UnconfirmedTransactions();
			});
		}
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_TooSmallPacketIsRejected) {
		// Arrange:
		ionet::Packet packet;
		packet.Size = sizeof(ionet::Packet) - 1;
		packet.Type = ionet::PacketType::Pull_Transactions;
		ionet::ServerPacketHandlers handlers;
		size_t counter = 0;
		RegisterHandler(handlers, counter);

		// Act:
		ionet::ServerPacketHandlerContext context;
		EXPECT_TRUE(handlers.process(packet, context));

		// Assert:
		EXPECT_EQ(0u, counter);
		test::AssertNoResponse(context);
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_PacketWithWrongTypeIsRejected) {
		// Arrange:
		ionet::Packet packet;
		packet.Type = ionet::PacketType::Pull_Blocks;
		ionet::ServerPacketHandlers handlers;
		size_t counter = 0;
		RegisterHandler(handlers, counter);

		// Act:
		ionet::ServerPacketHandlerContext context;
		EXPECT_FALSE(handlers.process(packet, context));

		// Assert:
		EXPECT_EQ(0u, counter);
		test::AssertNoResponse(context);
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_PacketWithInvalidPayloadIsRejected) {
		// Arrange: payload size is not divisible by sizeof(uint32_t)
		auto pPacket = CreatePacket(3);
		ionet::ServerPacketHandlers handlers;
		size_t counter = 0;
		RegisterHandler(handlers, counter);

		// Act:
		ionet::ServerPacketHandlerContext context;
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert:
		EXPECT_EQ(0u, counter);
		test::AssertNoResponse(context);
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_ValidPacketWithNoPayloadIsAccepted) {
		// Arrange:
		auto pPacket = CreatePacket(0);
		ionet::ServerPacketHandlers handlers;
		size_t counter = 0;
		RegisterHandler(handlers, counter);

		// Act:
		ionet::ServerPacketHandlerContext context;
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert:
		EXPECT_EQ(1u, counter);
		EXPECT_TRUE(context.hasResponse());

		const auto& payload = context.response();
		test::AssertPacketHeader(payload, sizeof(ionet::Packet), ionet::PacketType::Pull_Transactions);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_ValidPacketWithNonEmptyPayloadIsAccepted) {
		// Arrange:
		auto pPacket = CreatePacket(3 * sizeof(uint32_t));
		ionet::ServerPacketHandlers handlers;
		size_t counter = 0;
		RegisterHandler(handlers, counter);

		// Act:
		ionet::ServerPacketHandlerContext context;
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert:
		EXPECT_EQ(1u, counter);
		EXPECT_TRUE(context.hasResponse());
		const auto& payload = context.response();
		test::AssertPacketHeader(payload, sizeof(ionet::Packet), ionet::PacketType::Pull_Transactions);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		UnconfirmedTransactions CreateTransactions(size_t numTransactions) {
			UnconfirmedTransactions transactions;
			for (uint16_t i = 0u; i < numTransactions; ++i)
				transactions.push_back(mocks::CreateMockTransaction(i + 1));

			return transactions;
		}

		void AssertResponseIsSetIfPacketIsValid(size_t numTransactions) {
			// Arrange:
			constexpr size_t numShortHashes = 3;
			auto pPacket = CreatePacket(numShortHashes * sizeof(uint32_t));
			ionet::ServerPacketHandlers handlers;
			size_t counter = 0;

			utils::ShortHashesSet extractedShortHashes;
			auto pData = reinterpret_cast<utils::ShortHash*>(pPacket->Data());
			for (auto i = 0u; i < numShortHashes; ++i)
				extractedShortHashes.insert(*pData++);

			auto expectedTransactions = CreateTransactions(numTransactions);
			utils::ShortHashesSet actualShortHashes;
			RegisterPullTransactionsHandler(handlers, [&](const auto& shortHashes) {
				++counter;
				actualShortHashes = shortHashes;
				return expectedTransactions;
			});

			// Act:
			ionet::ServerPacketHandlerContext context;
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert:
			EXPECT_EQ(1u, counter);
			EXPECT_TRUE(context.hasResponse());
			auto payload = context.response();
			test::AssertPacketHeader(
					payload,
					sizeof(ionet::Packet) + test::TotalSize(expectedTransactions),
					ionet::PacketType::Pull_Transactions);
			ASSERT_EQ(numTransactions, payload.buffers().size());

			auto i = 0u;
			for (const auto& pExpectedTransaction : expectedTransactions) {
				auto pTransaction = reinterpret_cast<const mocks::MockTransaction*>(payload.buffers()[i++].pData);
				EXPECT_EQ(*pExpectedTransaction, *pTransaction);
			}

			EXPECT_EQ(extractedShortHashes, actualShortHashes);
		}
	}

	TEST(TransactionHandlersTests, PushTransactionsHandler_ResponseIsSetIfPacketIsValid) {
		// Assert:
		AssertResponseIsSetIfPacketIsValid(0);
		AssertResponseIsSetIfPacketIsValid(1);
		AssertResponseIsSetIfPacketIsValid(3);
		AssertResponseIsSetIfPacketIsValid(10);
	}

	// endregion
}}
