#include "catapult/local/BroadcastUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

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
				ionet::PacketType expectedPacketType) {
			// Act:
			auto payload = CreateBroadcastPayload(payloadSeed);

			// Assert:
			test::AssertPacketHeader(payload, sizeof(ionet::PacketHeader) + pEntity->Size, expectedPacketType);
			ASSERT_EQ(1u, payload.buffers().size());

			// - the buffer contains the correct data and points to the original entity
			AssertEntityBuffer(payload.buffers()[0], *pEntity);
		}
	}

	// region block

	TEST(BroadcastUtilsTests, CanCreateBroadcastPayload_Block_Single) {
		// Arrange:
		auto pBlock = std::shared_ptr<model::Block>(test::GenerateEmptyRandomBlock());

		// Assert:
		AssertCanCreateSingleEntityBroadcastPayload(pBlock, pBlock, ionet::PacketType::Push_Block);
	}

	// endregion

	// region transaction infos

	TEST(BroadcastUtilsTests, CanCreateBroadcastPayload_TransactionInfos_None) {
		// Arrange:
		std::vector<model::TransactionInfo> infos;

		// Act:
		auto payload = CreateBroadcastPayload(infos);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(ionet::PacketHeader), ionet::PacketType::Push_Transactions);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		model::TransactionInfo CreateRandomTransactionInfo() {
			auto pTransaction = std::shared_ptr<model::Transaction>(test::GenerateRandomTransaction());
			return model::TransactionInfo(pTransaction);
		}
	}

	TEST(BroadcastUtilsTests, CanCreateBroadcastPayload_TransactionInfos_Single) {
		// Arrange:
		std::vector<model::TransactionInfo> infos;
		infos.push_back(CreateRandomTransactionInfo());

		// Assert:
		AssertCanCreateSingleEntityBroadcastPayload(infos, infos[0].pEntity, ionet::PacketType::Push_Transactions);
	}

	TEST(BroadcastUtilsTests, CanCreateBroadcastPayload_TransactionInfos_Multiple) {
		// Arrange:
		std::vector<model::TransactionInfo> infos;
		infos.push_back(CreateRandomTransactionInfo());
		infos.push_back(CreateRandomTransactionInfo());

		// Act:
		auto payload = CreateBroadcastPayload(infos);

		// Assert:
		test::AssertPacketHeader(
				payload,
				sizeof(ionet::PacketHeader) + infos[0].pEntity->Size + infos[1].pEntity->Size,
				ionet::PacketType::Push_Transactions);
		ASSERT_EQ(2u, payload.buffers().size());

		// - each buffer contains the correct data and points to the original entities
		for (auto i = 0u; i < payload.buffers().size(); ++i)
			AssertEntityBuffer(payload.buffers()[i], *infos[i].pEntity);
	}

	// endregion
}}
