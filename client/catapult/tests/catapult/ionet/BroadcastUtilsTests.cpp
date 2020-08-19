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

#include "catapult/ionet/BroadcastUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS BroadcastUtilsTests

	namespace {
		void AssertEntityBuffer(RawBuffer buffer, const model::VerifiableEntity& entity) {
			EXPECT_EQ(test::AsVoidPointer(&entity), buffer.pData);
			ASSERT_EQ(entity.Size, buffer.Size);
			EXPECT_EQ(entity, *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
		}

		template<typename TPayloadSeed, typename TEntity>
		void AssertCanCreateSingleEntityBroadcastPayload(
				const TPayloadSeed& payloadSeed,
				const std::shared_ptr<TEntity>& pEntity,
				PacketType expectedPacketType) {
			// Act:
			auto payload = CreateBroadcastPayload(payloadSeed);

			// Assert:
			test::AssertPacketHeader(payload, sizeof(PacketHeader) + pEntity->Size, expectedPacketType);
			ASSERT_EQ(1u, payload.buffers().size());

			// - the buffer contains the correct data and points to the original entity
			AssertEntityBuffer(payload.buffers()[0], *pEntity);
		}
	}

	// region block

	TEST(TEST_CLASS, CanCreateBroadcastPayload_Block_Single) {
		// Arrange:
		auto pBlock = std::shared_ptr<model::Block>(test::GenerateEmptyRandomBlock());

		// Assert:
		AssertCanCreateSingleEntityBroadcastPayload(pBlock, pBlock, PacketType::Push_Block);
	}

	// endregion

	// region transaction infos

	TEST(TEST_CLASS, CanCreateBroadcastPayload_TransactionInfos_None) {
		// Arrange:
		std::vector<model::TransactionInfo> transactionInfos;

		// Act:
		auto payload = CreateBroadcastPayload(transactionInfos);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Push_Transactions);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		model::TransactionInfo CreateRandomTransactionInfo() {
			auto pTransaction = std::shared_ptr<model::Transaction>(test::GenerateRandomTransaction());
			return model::TransactionInfo(pTransaction);
		}
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_TransactionInfos_Single) {
		// Arrange:
		std::vector<model::TransactionInfo> transactionInfos;
		transactionInfos.push_back(CreateRandomTransactionInfo());

		// Assert:
		AssertCanCreateSingleEntityBroadcastPayload(transactionInfos, transactionInfos[0].pEntity, PacketType::Push_Transactions);
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_TransactionInfos_Multiple) {
		// Arrange:
		std::vector<model::TransactionInfo> transactionInfos;
		transactionInfos.push_back(CreateRandomTransactionInfo());
		transactionInfos.push_back(CreateRandomTransactionInfo());

		// Act:
		auto payload = CreateBroadcastPayload(transactionInfos);

		// Assert:
		test::AssertPacketHeader(
				payload,
				sizeof(PacketHeader) + transactionInfos[0].pEntity->Size + transactionInfos[1].pEntity->Size,
				PacketType::Push_Transactions);
		ASSERT_EQ(2u, payload.buffers().size());

		// - each buffer contains the correct data and points to the original entities
		for (auto i = 0u; i < payload.buffers().size(); ++i)
			AssertEntityBuffer(payload.buffers()[i], *transactionInfos[i].pEntity);
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_TransactionInfos_CustomPacketType) {
		// Arrange:
		std::vector<model::TransactionInfo> transactionInfos;
		transactionInfos.push_back(CreateRandomTransactionInfo());

		// Act:
		auto payload = CreateBroadcastPayload(transactionInfos, PacketType::Chain_Statistics);

		// Assert:
		const auto& entity = *transactionInfos[0].pEntity;
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + entity.Size, PacketType::Chain_Statistics);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer contains the correct data and points to the original entity
		AssertEntityBuffer(payload.buffers()[0], entity);
	}

	// endregion

	// region cosignatures

	namespace {
		void AssertPayloadBuffer(const PacketPayload& payload, const std::vector<model::DetachedCosignature>& cosignatures) {
			// Assert:
			auto cosignatureSize = sizeof(model::DetachedCosignature);
			auto expectedPayloadSize = cosignatures.size() * cosignatureSize;
			test::AssertPacketHeader(payload, sizeof(PacketHeader) + expectedPayloadSize, PacketType::Push_Detached_Cosignatures);

			// - a single buffer is present composed of all cosignatures
			ASSERT_EQ(1u, payload.buffers().size());

			const auto& buffer = payload.buffers()[0];
			ASSERT_EQ(expectedPayloadSize, buffer.Size);

			// - all cosignatures are present in the buffer
			const auto* pCosignature = reinterpret_cast<const model::DetachedCosignature*>(buffer.pData);
			for (auto i = 0u; i < cosignatures.size(); ++i, ++pCosignature)
				EXPECT_EQ_MEMORY(cosignatures.data() + i, pCosignature, cosignatureSize) << "cosignature at " << i;
		}
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_Cosignatures_None) {
		// Arrange:
		std::vector<model::DetachedCosignature> infos;

		// Act:
		auto payload = CreateBroadcastPayload(infos);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Push_Detached_Cosignatures);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_Cosignatures_Single) {
		// Arrange:
		std::vector<model::DetachedCosignature> cosignatures;
		cosignatures.push_back(test::CreateRandomDetachedCosignature());

		// Act:
		auto payload = CreateBroadcastPayload(cosignatures);

		// Assert:
		AssertPayloadBuffer(payload, cosignatures);
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_Cosignatures_Multiple) {
		// Arrange:
		std::vector<model::DetachedCosignature> cosignatures;
		for (auto i = 0u; i < 5; ++i)
			cosignatures.push_back(test::CreateRandomDetachedCosignature());

		// Act:
		auto payload = CreateBroadcastPayload(cosignatures);

		// Assert:
		AssertPayloadBuffer(payload, cosignatures);
	}

	// endregion
}}
