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

#include "partialtransaction/src/handlers/PtHandlers.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "catapult/utils/Functional.h"
#include "tests/test/core/PushHandlerTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/PullHandlerTests.h"
#include "tests/TestHarness.h"

using namespace catapult::partialtransaction;

namespace catapult { namespace handlers {

#define TEST_CLASS PtHandlersTests

	// region PushPartialTransactionsHandler

	namespace {
		struct PushPtTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Push_Partial_Transactions;
			static constexpr auto Data_Size = sizeof(mocks::MockTransaction);

			static constexpr size_t AdditionalPacketSize(size_t numTransactions) {
				return numTransactions * (numTransactions + 1) / 2;
			}

			static void PreparePacket(ionet::ByteBuffer& buffer, size_t count) {
				auto currentOffset = sizeof(ionet::Packet);
				for (auto i = 0u; i < count; ++i) {
					auto size = Data_Size + i + 1;
					test::SetTransactionAt(buffer, currentOffset, size);
					reinterpret_cast<model::VerifiableEntity&>(buffer[currentOffset]).Type = model::Entity_Type_Aggregate_Bonded;
					currentOffset += size;
				}
			}

			static auto CreateRegistry() {
				model::TransactionRegistry registry;
				registry.registerPlugin(mocks::CreateMockTransactionPlugin(model::Entity_Type_Aggregate_Bonded));
				registry.registerPlugin(mocks::CreateMockTransactionPlugin());
				return registry;
			}

			static auto RegisterHandler(
					ionet::ServerPacketHandlers& handlers,
					const model::TransactionRegistry& registry,
					const TransactionRangeHandler& rangeHandler) {
				return RegisterPushPartialTransactionsHandler(handlers, registry, rangeHandler);
			}
		};
	}

	DEFINE_PUSH_HANDLER_TESTS(TEST_CLASS, PushPt)

	TEST(TEST_CLASS, PushTransactionsHandler_ValidPacketWithUnhandledTransactionsIsRejected) {
		// Arrange: malform type of middle transaction, transactions have sizes: Data_Size + 1, Data_Size + 2, etc.
		test::PushHandlerTests<PushPtTraits>::PushHandlerBuffer buffer(3, true);
		auto dataOffset = sizeof(ionet::Packet);
		auto tx1Size = PushPtTraits::Data_Size + 1;
		reinterpret_cast<model::VerifiableEntity&>(buffer.buffer()[dataOffset + tx1Size]).Type = mocks::MockTransaction::Entity_Type;

		// Act:
		test::PushHandlerTests<PushPtTraits>::RunPushHandlerTest(PushPtTraits::RegisterHandler, buffer.packet(), [](const auto& counters) {
			// Assert:
			EXPECT_TRUE(counters.empty());
		});
	}

	// endregion

	// region PullPartialTransactionInfosHandler - basic edge case tests

	namespace {
		struct PullTransactionsTraits {
			static constexpr auto Data_Header_Size = 0u;
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Partial_Transaction_Infos;
			static constexpr auto RegisterHandler = RegisterPullPartialTransactionInfosHandler;
			static constexpr auto Valid_Request_Payload_Size = SizeOf32<cache::ShortHashPair>();

			using ResponseType = CosignedTransactionInfos;
			using RetrieverParamType = cache::ShortHashPairMap;
		};
	}

	DEFINE_PULL_HANDLER_EDGE_CASE_TESTS(TEST_CLASS, PullTransactions)

	// endregion

	// region PullPartialTransactionInfosHandler - request + response tests

	namespace {
		auto ExtractFromPacket(const ionet::Packet& packet, size_t numRequestHashPairs) {
			cache::ShortHashPairMap extractedMap;
			const auto* pData = reinterpret_cast<const cache::ShortHashPair*>(packet.Data());
			for (auto i = 0u; i < numRequestHashPairs; ++i) {
				extractedMap.emplace(pData->TransactionShortHash, pData->CosignaturesShortHash);
				++pData;
			}

			return extractedMap;
		}

		class PullResponseContext {
		public:
			explicit PullResponseContext(size_t numResponseTransactions) {
				// note: 0th element will have 0 cosignatures
				for (uint16_t i = 0u; i < numResponseTransactions; ++i) {
					m_transactionInfos.push_back({
						test::GenerateRandomByteArray<Hash256>(),
						i % 2 ? nullptr : mocks::CreateMockTransaction(static_cast<uint16_t>(i)),
						test::GenerateRandomDataVector<model::Cosignature>(i)
					});
				}
			}

		public:
			const auto& response() const {
				return m_transactionInfos;
			}

			auto responseSize() const {
				return utils::Sum(m_transactionInfos, [](const auto& transactionInfo) {
					size_t size = sizeof(uint64_t);
					size += transactionInfo.pTransaction
							? transactionInfo.pTransaction->Size + utils::GetPaddingSize(transactionInfo.pTransaction->Size, 8)
							: Hash256::Size;
					size += transactionInfo.Cosignatures.size() * sizeof(model::Cosignature);
					return size;
				});
			}

			void assertPayload(const ionet::PacketPayload& payload) {
				// number of buffers is dependent on transactionInfo
				auto expectedNumBuffers = utils::Sum(m_transactionInfos, [](const auto& transactionInfo) {
					size_t numBuffers = 2;
					if (transactionInfo.pTransaction && 0 != utils::GetPaddingSize(transactionInfo.pTransaction->Size, 8))
						++numBuffers;

					if (!transactionInfo.Cosignatures.empty())
						++numBuffers;

					return numBuffers;
				});
				ASSERT_EQ(expectedNumBuffers, payload.buffers().size());

				auto i = 0u;
				for (auto infoIndex = 0u; infoIndex < m_transactionInfos.size(); ++infoIndex) {
					const auto& transactionInfo = m_transactionInfos[infoIndex];
					auto failedMessage = " for info " + std::to_string(infoIndex);

					auto tagValue = reinterpret_cast<const uint64_t&>(*payload.buffers()[i++].pData);
					uint64_t expectedTagValue = static_cast<uint16_t>(transactionInfo.Cosignatures.size());
					expectedTagValue |= transactionInfo.pTransaction ? 0x8000 : 0;
					EXPECT_EQ(expectedTagValue, tagValue) << failedMessage;

					if (transactionInfo.pTransaction) {
						const auto& transaction = reinterpret_cast<const mocks::MockTransaction&>(*payload.buffers()[i++].pData);
						EXPECT_EQ(*transactionInfo.pTransaction, transaction) << failedMessage;

						auto paddingSize = utils::GetPaddingSize(transactionInfo.pTransaction->Size, 8);
						if (0 != paddingSize) {
							auto zeros = std::vector<uint8_t>(paddingSize, 0);
							auto paddingBuffer = payload.buffers()[i++];
							EXPECT_EQ_MEMORY(zeros.data(), paddingBuffer.pData, paddingBuffer.Size);
						}
					} else {
						const auto& hash = reinterpret_cast<const Hash256&>(*payload.buffers()[i++].pData);
						EXPECT_EQ(transactionInfo.EntityHash, hash) << failedMessage;
					}

					if (!transactionInfo.Cosignatures.empty()) {
						const auto* pCosignatures = payload.buffers()[i++].pData;
						auto expectedSize = transactionInfo.Cosignatures.size() * sizeof(model::Cosignature);
						EXPECT_EQ_MEMORY(transactionInfo.Cosignatures.data(), pCosignatures, expectedSize) << failedMessage;
					}
				}
			}

		private:
			CosignedTransactionInfos m_transactionInfos;
		};

		void AssertPullResponseIsSetWhenPacketIsValid(uint32_t numRequestHashPairs, uint32_t numResponseTransactions) {
			// Arrange:
			auto packetType = PullTransactionsTraits::Packet_Type;
			auto pPacket = test::CreateRandomPacket(numRequestHashPairs * SizeOf32<cache::ShortHashPair>(), packetType);
			ionet::ServerPacketHandlers handlers;
			size_t counter = 0;

			auto extractedRequestHashPairs = ExtractFromPacket(*pPacket, numRequestHashPairs);
			cache::ShortHashPairMap actualRequestHashPairs;
			PullResponseContext responseContext(numResponseTransactions);
			RegisterPullPartialTransactionInfosHandler(handlers, [&](const auto& requestHashPairs) {
				++counter;
				actualRequestHashPairs = requestHashPairs;
				return responseContext.response();
			});

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: the requested hash pairs were passed to the supplier
			EXPECT_EQ(extractedRequestHashPairs, actualRequestHashPairs);

			// - the handler was called and has the correct header
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(handlerContext.hasResponse());
			auto payload = handlerContext.response();
			test::AssertPacketHeader(payload, sizeof(ionet::PacketHeader) + responseContext.responseSize(), packetType);

			// - let the traits assert the returned payload (may be one or more buffers)
			responseContext.assertPayload(payload);
		}
	}

	DEFINE_PULL_HANDLER_REQUEST_RESPONSE_TESTS(TEST_CLASS, PullTransactions, AssertPullResponseIsSetWhenPacketIsValid)

	// endregion
}}
