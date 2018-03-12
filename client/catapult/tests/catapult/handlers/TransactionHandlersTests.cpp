#include "catapult/handlers/TransactionHandlers.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/PushHandlerTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/PullHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS TransactionHandlersTests

	// region PushTransactionsHandler

	namespace {
		struct PushTransactionsTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Push_Transactions;
			static constexpr auto Data_Size = sizeof(mocks::MockTransaction);

			static constexpr size_t AdditionalPacketSize(size_t numTransactions) {
				return numTransactions * (numTransactions + 1) / 2;
			}

			static void PreparePacket(ionet::ByteBuffer& buffer, size_t count) {
				auto currentOffset = sizeof(ionet::Packet);
				for (auto i = 0u; i < count; ++i) {
					auto size = Data_Size + i + 1;
					test::SetTransactionAt(buffer, currentOffset, size);
					currentOffset += size;
				}
			}

			static auto CreateRegistry() {
				model::TransactionRegistry registry;
				registry.registerPlugin(mocks::CreateMockTransactionPlugin());
				return registry;
			}

			static auto RegisterHandler(
					ionet::ServerPacketHandlers& handlers,
					const model::TransactionRegistry& registry,
					const TransactionRangeHandler& rangeHandler) {
				return RegisterPushTransactionsHandler(handlers, registry, rangeHandler);
			}
		};
	}

	DEFINE_PUSH_HANDLER_TESTS(TEST_CLASS, PushTransactions)

	// endregion

	// region PullTransactionsHandler

	namespace {
		struct PullTransactionsTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Transactions;
			static constexpr auto Register_Handler_Func = RegisterPullTransactionsHandler;
			using ResponseType = UnconfirmedTransactions;
			static constexpr auto Valid_Request_Payload_Size = sizeof(utils::ShortHash);

			using RetrieverParamType = utils::ShortHashesSet;

			static auto ExtractFromPacket(const ionet::Packet& packet, size_t numRequestEntities) {
				RetrieverParamType extracted;
				auto pData = reinterpret_cast<const utils::ShortHash*>(packet.Data());
				for (auto i = 0u; i < numRequestEntities; ++i)
					extracted.insert(*pData++);

				return extracted;
			}

			class ResponseContext {
			public:
				explicit ResponseContext(size_t numResponseEntities) {
					for (uint16_t i = 0u; i < numResponseEntities; ++i)
						m_transactions.push_back(mocks::CreateMockTransaction(i + 1));
				}

				const auto& response() const {
					return m_transactions;
				}

				auto responseSize() const {
					return test::TotalSize(m_transactions);
				}

				void assertPayload(const ionet::PacketPayload& payload) {
					ASSERT_EQ(m_transactions.size(), payload.buffers().size());

					auto i = 0u;
					for (const auto& pExpectedTransaction : m_transactions) {
						const auto& transaction = reinterpret_cast<const mocks::MockTransaction&>(*payload.buffers()[i++].pData);
						EXPECT_EQ(*pExpectedTransaction, transaction);
					}
				}

			private:
				UnconfirmedTransactions m_transactions;
			};
		};
	}

	DEFINE_PULL_HANDLER_TESTS(TEST_CLASS, PullTransactions)

	// endregion
}}
