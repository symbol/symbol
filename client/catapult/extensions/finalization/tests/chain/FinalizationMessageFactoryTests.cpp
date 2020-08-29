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

#include "finalization/src/chain/FinalizationMessageFactory.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/crypto_voting/OtsTree.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationMessageFactoryTests

	// region TestContext

	namespace {
		class TestContext {
		public:
			static constexpr auto Ots_Key_Dilution = 13u;

		public:
			TestContext(FinalizationPoint point, Height height)
					: TestContext(point, height, 10, finalization::FinalizationConfiguration::Uninitialized())
			{}

			TestContext(FinalizationPoint point, Height height, uint32_t numBlocks, const finalization::FinalizationConfiguration& config)
					: m_lastFinalizedHash(test::GenerateRandomByteArray<Hash256>())
					, m_pBlockStorage(mocks::CreateMemoryBlockStorageCache(numBlocks))
					, m_proofStorage(std::make_unique<mocks::MockProofStorage>(point, height, m_lastFinalizedHash))
					, m_pFactory(CreateFinalizationMessageFactory(
							config,
							*m_pBlockStorage,
							m_proofStorage,
							CreateOtsTree(m_otsTreeStream, point)))
			{}

		public:
			auto& factory() {
				return *m_pFactory;
			}

		public:
			Hash256 lastFinalizedHash() const {
				return m_lastFinalizedHash;
			}

			Hash256 blockHashAt(Height height) const {
				return m_pBlockStorage->view().loadBlockElement(height)->EntityHash;
			}

		private:
			static crypto::OtsTree CreateOtsTree(io::SeekableStream& storage, FinalizationPoint point) {
				auto startKeyIdentifier = model::StepIdentifierToOtsKeyIdentifier(
						{ point, model::FinalizationStage::Prevote },
						Ots_Key_Dilution);
				auto endKeyIdentifier = model::StepIdentifierToOtsKeyIdentifier(
						{ point + FinalizationPoint(20), model::FinalizationStage::Precommit },
						Ots_Key_Dilution);
				return crypto::OtsTree::Create(
						test::GenerateKeyPair(),
						storage,
						{ Ots_Key_Dilution, startKeyIdentifier, endKeyIdentifier });
			}

		private:
			Hash256 m_lastFinalizedHash;
			std::unique_ptr<io::BlockStorageCache> m_pBlockStorage;
			io::ProofStorageCache m_proofStorage;
			mocks::MockSeekableMemoryStream m_otsTreeStream;

			std::unique_ptr<FinalizationMessageFactory> m_pFactory;
		};

		bool IsSigned(const model::FinalizationMessage& message) {
			auto dilution = TestContext::Ots_Key_Dilution;
			auto keyIdentifier = model::StepIdentifierToOtsKeyIdentifier(message.StepIdentifier, dilution);
			return crypto::Verify(message.Signature, keyIdentifier, {
				reinterpret_cast<const uint8_t*>(&message) + model::FinalizationMessage::Header_Size,
				message.Size - model::FinalizationMessage::Header_Size
			});
		}
	}

	// endregion

	// region createPrevote

	namespace {
		void AssertPrevote(
				const model::FinalizationMessage& message,
				const TestContext& context,
				FinalizationPoint expectedPoint,
				Height expectedStartHeight,
				uint32_t expectedHashesCount) {
			EXPECT_EQ(sizeof(model::FinalizationMessage) + expectedHashesCount * Hash256::Size, message.Size);
			ASSERT_EQ(expectedHashesCount, message.HashesCount);

			EXPECT_EQ(model::StepIdentifier({ expectedPoint, model::FinalizationStage::Prevote }), message.StepIdentifier);
			EXPECT_EQ(expectedStartHeight, message.Height);
			for (auto i = 0u; i < expectedHashesCount; ++i)
				EXPECT_EQ(context.blockHashAt(expectedStartHeight + Height(i)), message.HashesPtr()[i]);

			EXPECT_TRUE(IsSigned(message));
		}

		void AssertCanCreatePrevote(
				uint32_t numBlocks,
				uint32_t maxHashesPerPoint,
				uint16_t prevoteBlocksMultiple,
				uint32_t expectedHashesCount) {
			// Arrange:
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			config.MaxHashesPerPoint = maxHashesPerPoint;
			config.PrevoteBlocksMultiple = prevoteBlocksMultiple;
			config.VotingSetGrouping = 500;

			TestContext context(FinalizationPoint(11), Height(8), numBlocks, config);

			// Act:
			auto pMessage = context.factory().createPrevote(FinalizationPoint(20));

			// Assert:
			AssertPrevote(*pMessage, context, FinalizationPoint(20), Height(8), expectedHashesCount);
		}
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainIsMissingFinalizedBlocks) {
		// Arrange:
		auto config = finalization::FinalizationConfiguration::Uninitialized();
		config.MaxHashesPerPoint = 10;
		config.PrevoteBlocksMultiple = 2;
		config.VotingSetGrouping = 500;

		TestContext context(FinalizationPoint(11), Height(8), 6, config);

		// Act:
		auto pMessage = context.factory().createPrevote(FinalizationPoint(20));

		// Assert:
		EXPECT_EQ(sizeof(model::FinalizationMessage) + Hash256::Size, pMessage->Size);
		ASSERT_EQ(1u, pMessage->HashesCount);

		EXPECT_EQ(model::StepIdentifier({ FinalizationPoint(20), model::FinalizationStage::Prevote }), pMessage->StepIdentifier);
		EXPECT_EQ(Height(8), pMessage->Height);
		EXPECT_EQ(context.lastFinalizedHash(), pMessage->HashesPtr()[0]);

		EXPECT_TRUE(IsSigned(*pMessage));
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainIsFullyFinalized_OnMultiple) {
		AssertCanCreatePrevote(8, 10, 2, 1);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainIsFullyFinalized_NotOnMultiple) {
		// Assert: even with multiple of 5, hash of last finalized block should be returned
		AssertCanCreatePrevote(8, 10, 5, 1);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasUnfinalizedBlocks_OnMultiple) {
		AssertCanCreatePrevote(12, 10, 2, 5);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasUnfinalizedBlocks_NotOnMultiple) {
		AssertCanCreatePrevote(12, 10, 5, 3);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasGreaterThanMaxUnfinalizedBlocks_OnMultiple) {
		AssertCanCreatePrevote(22, 10, 1, 10);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasGreaterThanMaxUnfinalizedBlocks_NotOnMultiple) {
		AssertCanCreatePrevote(22, 10, 5, 8);
	}

	// endregion

	// region createPrevote - VotingSetGrouping

	namespace {
		void AssertCanCreatePrevoteHonorsVotingSetGrouping(Height height, uint32_t expectedHashesCount) {
			// Arrange:
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			config.MaxHashesPerPoint = 400;
			config.PrevoteBlocksMultiple = 1;
			config.VotingSetGrouping = 50;

			TestContext context(FinalizationPoint(11), height, 200, config);

			// Act:
			auto pMessage = context.factory().createPrevote(FinalizationPoint(20));

			// Assert:
			AssertPrevote(*pMessage, context, FinalizationPoint(20), height, expectedHashesCount);
		}
	}

	TEST(TEST_CLASS, CanCreatePrevoteContainingAllVotingSetGroupHashes) {
		// Assert: message is limited to hashes in a single voting set
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(1), 50);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(51), 50);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(101), 50);
	}

	TEST(TEST_CLASS, CanCreatePrevoteContainingPartialVotingSetGroupHashes) {
		// Assert: message is limited to hashes in a single voting set
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(26), 25);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(76), 25);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(126), 25);
	}

	TEST(TEST_CLASS, CanCreatePrevoteContainingOnlyLastVotingSetGroupHash) {
		// Assert: message is limited to hashes in a single voting set
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(50), 1);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(100), 1);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(Height(150), 1);
	}

	// endregion

	// region createPrecommit

	TEST(TEST_CLASS, CanCreatePrecommit) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context(FinalizationPoint(11), Height(7));

		// Act:
		auto pMessage = context.factory().createPrecommit(FinalizationPoint(20), Height(35), hash);

		// Assert:
		EXPECT_EQ(sizeof(model::FinalizationMessage) + Hash256::Size, pMessage->Size);
		ASSERT_EQ(1u, pMessage->HashesCount);

		EXPECT_EQ(model::StepIdentifier({ FinalizationPoint(20), model::FinalizationStage::Precommit }), pMessage->StepIdentifier);
		EXPECT_EQ(Height(35), pMessage->Height);
		EXPECT_EQ(hash, pMessage->HashesPtr()[0]);

		EXPECT_TRUE(IsSigned(*pMessage));
	}

	// endregion
}}
