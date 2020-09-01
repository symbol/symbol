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

	// region PullTransactionsHandler - basic edge case tests

	namespace {
		struct PullTransactionsTraits {
			static constexpr auto Data_Header_Size = sizeof(BlockFeeMultiplier);
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Transactions;
			static constexpr auto Valid_Request_Payload_Size = SizeOf32<utils::ShortHash>();

			using ResponseType = UnconfirmedTransactions;
			using RetrieverParamType = utils::ShortHashesSet;

			using UtRetrieverAdapter = std::function<UnconfirmedTransactions (const utils::ShortHashesSet&)>;
			static auto RegisterHandler(ionet::ServerPacketHandlers& handlers, const UtRetrieverAdapter& utRetriever) {
				handlers::RegisterPullTransactionsHandler(handlers, [utRetriever](auto, const auto& knownShortHashes){
					return utRetriever(knownShortHashes);
				});
			}
		};

		DEFINE_PULL_HANDLER_EDGE_CASE_TESTS(TEST_CLASS, PullTransactions)
	}

	// endregion

	// region PullTransactionsHandler - request + response tests

	namespace {
		struct PullTransactionsRequestResponseTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Transactions;
			static constexpr auto RegisterHandler = handlers::RegisterPullTransactionsHandler;

			using FilterType = BlockFeeMultiplier;

			class PullResponseContext {
			public:
				explicit PullResponseContext(size_t numResponseTransactions) {
					for (uint16_t i = 0u; i < numResponseTransactions; ++i)
						m_transactions.push_back(mocks::CreateMockTransaction(static_cast<uint16_t>(i + 1)));
				}

			public:
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

	DEFINE_PULL_HANDLER_REQUEST_RESPONSE_TESTS(
			TEST_CLASS,
			PullTransactions,
			test::PullEntitiesHandlerAssertAdapter<PullTransactionsRequestResponseTraits>::AssertFunc)

	// endregion
}}
