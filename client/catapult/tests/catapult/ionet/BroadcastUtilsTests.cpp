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
			EXPECT_EQ(entity.Size, buffer.Size);
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
		auto payload = CreateBroadcastPayload(transactionInfos, PacketType::Chain_Info);

		// Assert:
		const auto& entity = *transactionInfos[0].pEntity;
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + entity.Size, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer contains the correct data and points to the original entity
		AssertEntityBuffer(payload.buffers()[0], entity);
	}

	// endregion

	// region cosignatures

	namespace {
		void AssertPayloadBuffer(const PacketPayload& payload, const std::vector<model::DetachedCosignature>& cosignatures) {
			// Assert:
			test::AssertPacketHeader(
					payload,
					sizeof(PacketHeader) + cosignatures.size() * sizeof(model::DetachedCosignature),
					PacketType::Push_Detached_Cosignatures);
			ASSERT_EQ(cosignatures.size(), payload.buffers().size());

			// - each buffer has correct size and contains the correct data
			for (auto i = 0u; i < payload.buffers().size(); ++i) {
				const auto& buffer = payload.buffers()[i];
				ASSERT_EQ(sizeof(model::DetachedCosignature), buffer.Size);
				EXPECT_TRUE(0 == std::memcmp(cosignatures.data() + i, buffer.pData, buffer.Size));
			}
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
		cosignatures.push_back(test::CreateRandomCosignature());

		// Act:
		auto payload = CreateBroadcastPayload(cosignatures);

		// Assert:
		AssertPayloadBuffer(payload, cosignatures);
	}

	TEST(TEST_CLASS, CanCreateBroadcastPayload_Cosignatures_Multiple) {
		// Arrange:
		std::vector<model::DetachedCosignature> cosignatures;
		for (auto i = 0u; i < 5; ++i)
			cosignatures.push_back(test::CreateRandomCosignature());

		// Act:
		auto payload = CreateBroadcastPayload(cosignatures);

		// Assert:
		AssertPayloadBuffer(payload, cosignatures);
	}

	// endregion
}}
