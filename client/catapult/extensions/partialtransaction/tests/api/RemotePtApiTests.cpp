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

#include "partialtransaction/src/api/RemotePtApi.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/other/RemoteApiFactory.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

	namespace {
		using TransactionType = mocks::MockTransaction;

		std::shared_ptr<ionet::Packet> CreatePacketWithTransactionInfos(uint16_t numTransactions) {
			// Arrange: create transactions with variable (incrementing) sizes
			//          (each info in this test has two parts: (1) tag, (2) transaction)
			uint32_t payloadSize = numTransactions * SizeOf32<uint64_t>();
			for (uint16_t i = 0u; i < numTransactions; ++i) {
				uint32_t transactionSize = SizeOf32<TransactionType>() + i + 1;
				payloadSize += transactionSize + utils::GetPaddingSize(transactionSize, 8);
			}

			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
			test::FillWithRandomData({ pPacket->Data(), payloadSize });

			auto* pData = pPacket->Data();
			for (uint16_t i = 0u; i < numTransactions; ++i) {
				// - tag (transaction and no cosignatures)
				reinterpret_cast<uint64_t&>(*pData) = 0x8000;
				pData += sizeof(uint64_t);

				// - transaction
				uint32_t transactionSize = SizeOf32<TransactionType>() + i + 1;
				auto& transaction = reinterpret_cast<TransactionType&>(*pData);
				transaction.Size = transactionSize;
				transaction.Type = TransactionType::Entity_Type;
				transaction.Deadline = Timestamp(5 * i);
				transaction.Data.Size = static_cast<uint16_t>(i + 1);
				pData += transactionSize;

				// - padding
				auto paddingSize = utils::GetPaddingSize(transactionSize, 8);
				std::memset(pData, 0, paddingSize);
				pData += paddingSize;
			}

			return pPacket;
		}

		struct TransactionInfosTraits {
			static constexpr uint32_t Request_Data_Size = 3 * sizeof(cache::ShortHashPair);

			static std::vector<uint32_t> KnownHashesValues() {
				return { 123, 234, 345, 981, 431, 512 };
			}

			static cache::ShortHashPairRange KnownShortHashPairs() {
				// notice that the values from KnownHashesValues are "paired" up
				return cache::ShortHashPairRange::CopyFixed(reinterpret_cast<uint8_t*>(KnownHashesValues().data()), 3);
			}

			static auto Invoke(const RemotePtApi& api) {
				return api.transactionInfos(KnownShortHashPairs());
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = CreatePacketWithTransactionInfos(3);
				pResponsePacket->Type = ionet::PacketType::Pull_Partial_Transaction_Infos;
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// the packet is malformed because it has an incorrect tag specifying no transaction
				auto pResponsePacket = CreateValidResponsePacket();
				reinterpret_cast<uint16_t&>(*pResponsePacket->Data()) = 0x0000;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_EQ(ionet::PacketType::Pull_Partial_Transaction_Infos, packet.Type);
				ASSERT_EQ(sizeof(ionet::Packet) + Request_Data_Size, packet.Size);
				EXPECT_EQ_MEMORY(packet.Data(), KnownHashesValues().data(), Request_Data_Size);
			}

			static void ValidateResponse(
					const ionet::Packet& response,
					const partialtransaction::CosignedTransactionInfos& transactionInfos) {
				ASSERT_EQ(3u, transactionInfos.size());

				auto pExpectedData = response.Data();
				auto parsedIter = transactionInfos.cbegin();
				for (auto i = 0u; i < transactionInfos.size(); ++i) {
					std::string message = "comparing info at " + std::to_string(i);

					// - skip tag
					pExpectedData += sizeof(uint64_t);

					// - transaction
					const auto& expectedTransaction = reinterpret_cast<const TransactionType&>(*pExpectedData);
					const auto& actualTransaction = *parsedIter->pTransaction;
					ASSERT_EQ(expectedTransaction.Size, actualTransaction.Size) << message;
					EXPECT_EQ(Timestamp(5 * i), actualTransaction.Deadline) << message;
					EXPECT_EQ(expectedTransaction, actualTransaction) << message;
					pExpectedData += expectedTransaction.Size + utils::GetPaddingSize(expectedTransaction.Size, 8);

					// - hash and cosignatures
					EXPECT_EQ(Hash256(), parsedIter->EntityHash);
					EXPECT_TRUE(parsedIter->Cosignatures.empty());

					++parsedIter;
				}
			}
		};

		struct RemotePtApiTraits {
			static auto Create(ionet::PacketIo& packetIo, const model::NodeIdentity& remoteIdentity) {
				auto registry = mocks::CreateDefaultTransactionRegistry();
				return test::CreateLifetimeExtendedApi(CreateRemotePtApi, packetIo, remoteIdentity, std::move(registry));
			}

			static auto Create(ionet::PacketIo& packetIo) {
				return Create(packetIo, model::NodeIdentity());
			}
		};
	}

	DEFINE_REMOTE_API_TESTS(RemotePtApi)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_VALID(RemotePtApi, TransactionInfos)
}}
