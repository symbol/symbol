/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "finalization/src/io/PrevoteChainStorage.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
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
			explicit TestContext(Height height) : TestContext(height, 10, finalization::FinalizationConfiguration::Uninitialized())
			{}

			TestContext(Height height, uint32_t numBlocks, const finalization::FinalizationConfiguration& config)
					: m_lastFinalizedHash(test::GenerateRandomByteArray<Hash256>())
					, m_pBlockStorage(mocks::CreateMemoryBlockStorageCache(numBlocks))
					, m_proofStorage(std::make_unique<mocks::MockProofStorage>(FinalizationPoint(), height, m_lastFinalizedHash))
					, m_pFactory(CreateFinalizationMessageFactory(
							config,
							*m_pBlockStorage,
							m_proofStorage,
							[this](const auto&, const auto& descriptor) {
								m_capturedDescriptors.push_back(descriptor);
							},
							CreateAggregateBmPrivateKeyTree(m_bmPrivateKeyTreeStream)))
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

			const auto& capturedDescriptors() const {
				return m_capturedDescriptors;
			}

		private:
			static crypto::AggregateBmPrivateKeyTree CreateAggregateBmPrivateKeyTree(io::SeekableStream& storage) {
				return crypto::AggregateBmPrivateKeyTree([&storage]() {
					auto bmOptions = crypto::BmOptions{
						GetKeyIdentifier(FinalizationEpoch(), model::FinalizationStage::Prevote),
						GetKeyIdentifier(FinalizationEpoch(20), model::FinalizationStage::Precommit)
					};
					auto tree = crypto::BmPrivateKeyTree::Create(test::GenerateVotingKeyPair(), storage, bmOptions);
					return std::make_unique<crypto::BmPrivateKeyTree>(std::move(tree));
				});
			}

			static crypto::BmKeyIdentifier GetKeyIdentifier(FinalizationEpoch epoch, model::FinalizationStage stage) {
				return model::StepIdentifierToBmKeyIdentifier({ epoch, FinalizationPoint(), stage, });
			}

		private:
			Hash256 m_lastFinalizedHash;
			std::unique_ptr<io::BlockStorageCache> m_pBlockStorage;
			io::ProofStorageCache m_proofStorage;
			mocks::MockSeekableMemoryStream m_bmPrivateKeyTreeStream;
			std::vector<io::PrevoteChainDescriptor> m_capturedDescriptors;

			std::unique_ptr<FinalizationMessageFactory> m_pFactory;
		};

		bool IsSigned(const model::FinalizationMessage& message) {
			auto keyIdentifier = model::StepIdentifierToBmKeyIdentifier(message.Data().StepIdentifier);
			return crypto::Verify(message.Signature(), keyIdentifier, {
				reinterpret_cast<const uint8_t*>(&message) + model::FinalizationMessage::Header_Size,
				message.Size - model::FinalizationMessage::Header_Size
			});
		}
	}

	// endregion

	// region createPrevote

	namespace {
		constexpr auto Message_Size = model::FinalizationMessage::MinSize();

		void AssertPrevote(
				const model::FinalizationMessage& message,
				const TestContext& context,
				FinalizationEpoch expectedEpoch,
				FinalizationPoint expectedPoint,
				Height expectedStartHeight,
				uint32_t expectedHashesCount) {
			// Assert: message
			EXPECT_EQ(Message_Size + expectedHashesCount * Hash256::Size, message.Size);
			ASSERT_EQ(expectedHashesCount, message.Data().HashesCount);

			EXPECT_EQ(
					model::StepIdentifier({ expectedEpoch, expectedPoint, model::FinalizationStage::Prevote }),
					message.Data().StepIdentifier);
			EXPECT_EQ(expectedStartHeight, message.Data().Height);
			for (auto i = 0u; i < expectedHashesCount; ++i)
				EXPECT_EQ(context.blockHashAt(expectedStartHeight + Height(i)), message.HashesPtr()[i]);

			EXPECT_TRUE(IsSigned(message));

			// - descriptor
			ASSERT_EQ(1u, context.capturedDescriptors().size());
			const auto& descriptor = context.capturedDescriptors()[0];

			EXPECT_EQ(model::FinalizationRound({ expectedEpoch, expectedPoint }), descriptor.Round);
			EXPECT_EQ(expectedStartHeight, descriptor.Height);
			EXPECT_EQ(expectedHashesCount, descriptor.HashesCount);
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

			TestContext context(Height(8), numBlocks, config);

			// Act:
			auto pMessage = context.factory().createPrevote({ FinalizationEpoch(3), FinalizationPoint(20) });

			// Assert:
			AssertPrevote(*pMessage, context, FinalizationEpoch(3), FinalizationPoint(20), Height(8), expectedHashesCount);
		}
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainIsMissingFinalizedBlocks) {
		// Arrange:
		auto config = finalization::FinalizationConfiguration::Uninitialized();
		config.MaxHashesPerPoint = 10;
		config.PrevoteBlocksMultiple = 2;
		config.VotingSetGrouping = 500;

		TestContext context(Height(8), 6, config);

		// Act:
		auto pMessage = context.factory().createPrevote({ FinalizationEpoch(3), FinalizationPoint(20) });

		// Assert: message
		EXPECT_EQ(Message_Size + Hash256::Size, pMessage->Size);
		ASSERT_EQ(1u, pMessage->Data().HashesCount);

		EXPECT_EQ(test::CreateStepIdentifier(3, 20, model::FinalizationStage::Prevote), pMessage->Data().StepIdentifier);
		EXPECT_EQ(Height(8), pMessage->Data().Height);
		EXPECT_EQ(context.lastFinalizedHash(), pMessage->HashesPtr()[0]);

		EXPECT_TRUE(IsSigned(*pMessage));

		// - descriptor
		ASSERT_EQ(1u, context.capturedDescriptors().size());
		const auto& descriptor = context.capturedDescriptors()[0];

		EXPECT_EQ(test::CreateFinalizationRound(3, 20), descriptor.Round);
		EXPECT_EQ(Height(8), descriptor.Height);
		EXPECT_EQ(0u, descriptor.HashesCount);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainIsFullyFinalized_OnMultiple) {
		AssertCanCreatePrevote(8, 10, 2, 1);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainIsFullyFinalized_NotOnMultiple) {
		// Assert: even with multiple of 5, hash of last finalized block should be returned
		AssertCanCreatePrevote(8, 10, 5, 1);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasUnfinalizedBlocks_OnMultiple) {
		AssertCanCreatePrevote(16, 10, 2, 7); // 8, 9, 10, 11, 12, 13, 14
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasUnfinalizedBlocks_NotOnMultiple) {
		AssertCanCreatePrevote(16, 10, 5, 3); // 8, 9, 10
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasGreaterThanMaxUnfinalizedBlocks_OnMultiple) {
		AssertCanCreatePrevote(22, 10, 1, 10);
	}

	TEST(TEST_CLASS, CanCreatePrevoteWhenChainHasGreaterThanMaxUnfinalizedBlocks_NotOnMultiple) {
		AssertCanCreatePrevote(22, 10, 5, 8);
	}

	// endregion

	// region createPrevote - StartingEpoch (start height alignment)

	namespace {
		void AssertPrevoteStartingEpoch(uint64_t finalizedHeight, uint32_t chainHeight, uint32_t expectedHashesCount) {
			// Arrange:
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			config.MaxHashesPerPoint = 200;
			config.PrevoteBlocksMultiple = 5;
			config.VotingSetGrouping = 25;

			TestContext context(Height(finalizedHeight), chainHeight, config);

			// Act:
			auto pMessage = context.factory().createPrevote({ FinalizationEpoch(6), FinalizationPoint(1) });

			// Assert:
			AssertPrevote(*pMessage, context, FinalizationEpoch(6), FinalizationPoint(1), Height(finalizedHeight), expectedHashesCount);
		}
	}

	TEST(TEST_CLASS, CanCreatePrevote_BelowMultipleTimesTwo) {
		AssertPrevoteStartingEpoch(1, 4, 1);
		AssertPrevoteStartingEpoch(1, 5, 1);
		AssertPrevoteStartingEpoch(1, 6, 1);
		AssertPrevoteStartingEpoch(1, 9, 1);
	}

	TEST(TEST_CLASS, CanCreatePrevote_AboveMultipleTimesTwo) {
		AssertPrevoteStartingEpoch(1, 10, 5); // 1 - 5
		AssertPrevoteStartingEpoch(1, 11, 5); // 1 - 5
		AssertPrevoteStartingEpoch(1, 14, 5); // 1 - 5
		AssertPrevoteStartingEpoch(1, 15, 10); // 1 - 10
	}

	TEST(TEST_CLASS, CanCreatePrevoteStartingEpoch_BelowMultipleTimesTwo) {
		AssertPrevoteStartingEpoch(100, 104, 1);
		AssertPrevoteStartingEpoch(100, 105, 1);
		AssertPrevoteStartingEpoch(100, 106, 1);
		AssertPrevoteStartingEpoch(100, 109, 1);
	}

	TEST(TEST_CLASS, CanCreatePrevoteStartingEpoch_AboveMultipleTimesTwo) {
		AssertPrevoteStartingEpoch(100, 110, 6); // 100 - 105
		AssertPrevoteStartingEpoch(100, 111, 6); // 100 - 105
		AssertPrevoteStartingEpoch(100, 114, 6); // 100 - 105
		AssertPrevoteStartingEpoch(100, 115, 11); // 100 - 110
	}

	// endregion

	// region createPrevote - VotingSetGrouping

	namespace {
		void AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch epoch, Height height, uint32_t expectedHashesCount) {
			// Arrange:
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			config.MaxHashesPerPoint = 400;
			config.PrevoteBlocksMultiple = 1;
			config.VotingSetGrouping = 50;

			TestContext context(height, 200, config);

			// Act:
			auto pMessage = context.factory().createPrevote({ epoch, FinalizationPoint(20) });

			// Assert:
			AssertPrevote(*pMessage, context, epoch, FinalizationPoint(20), height, expectedHashesCount);
		}
	}

	TEST(TEST_CLASS, CanCreatePrevoteContainingAllVotingSetGroupHashes) {
		// Assert: message is limited to hashes in a single voting set
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(2), Height(1), 50);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(3), Height(51), 50);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(4), Height(101), 50);
	}

	TEST(TEST_CLASS, CanCreatePrevoteContainingPartialVotingSetGroupHashes) {
		// Assert: message is limited to hashes in a single voting set
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(2), Height(26), 25);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(3), Height(76), 25);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(4), Height(126), 25);
	}

	TEST(TEST_CLASS, CanCreatePrevoteContainingOnlyLastVotingSetGroupHash) {
		// Assert: message is limited to hashes in a single voting set
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(2), Height(50), 1);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(3), Height(100), 1);
		AssertCanCreatePrevoteHonorsVotingSetGrouping(FinalizationEpoch(4), Height(150), 1);
	}

	// endregion

	// region createPrecommit

	TEST(TEST_CLASS, CanCreatePrecommit) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context(Height(7));

		// Act:
		auto pMessage = context.factory().createPrecommit({ FinalizationEpoch(3), FinalizationPoint(20) }, Height(35), hash);

		// Assert:
		EXPECT_EQ(Message_Size + Hash256::Size, pMessage->Size);
		ASSERT_EQ(1u, pMessage->Data().HashesCount);

		EXPECT_EQ(test::CreateStepIdentifier(3, 20, model::FinalizationStage::Precommit), pMessage->Data().StepIdentifier);
		EXPECT_EQ(Height(35), pMessage->Data().Height);
		EXPECT_EQ(hash, pMessage->HashesPtr()[0]);

		EXPECT_TRUE(IsSigned(*pMessage));

		// - descriptor
		EXPECT_EQ(0u, context.capturedDescriptors().size());
	}

	// endregion
}}
