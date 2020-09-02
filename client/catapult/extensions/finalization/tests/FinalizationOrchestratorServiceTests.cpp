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

#include "finalization/src/FinalizationOrchestratorService.h"
#include "finalization/src/FinalizationBootstrapperService.h"
#include "finalization/src/FinalizationConfiguration.h"
#include "finalization/src/VotingStatusFile.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/crypto_voting/OtsTree.h"
#include "catapult/io/FileStream.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "finalization/tests/test/mocks/MockRoundMessageAggregator.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationOrchestratorServiceTests

	// region test context

	namespace {
		constexpr auto Num_Dependent_Services = 3u;
		constexpr auto Default_Voting_Set_Grouping = 500u;

		constexpr auto Finalization_Epoch = FinalizationEpoch(6);
		constexpr auto Prevote_Stage = model::FinalizationStage::Prevote;
		constexpr auto Precommit_Stage = model::FinalizationStage::Precommit;

		struct FinalizationOrchestratorServiceTraits {
			static auto CreateRegistrar(uint64_t votingSetGrouping) {
				// (Size, Threshold) are set in MockRoundMessageAggregator to (1000, 750)
				auto config = FinalizationConfiguration::Uninitialized();
				config.StepDuration = utils::TimeSpan::FromSeconds(10);
				config.MaxHashesPerPoint = 64;
				config.PrevoteBlocksMultiple = 5;
				config.VotingSetGrouping = votingSetGrouping;
				return CreateFinalizationOrchestratorServiceRegistrar(config);
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(Default_Voting_Set_Grouping);
			}
		};

		class TestContext : public test::ServiceLocatorTestContext<FinalizationOrchestratorServiceTraits> {
		private:
			using BaseType = test::ServiceLocatorTestContext<FinalizationOrchestratorServiceTraits>;

		private:
			static constexpr uint64_t Ots_Key_Dilution = 13;

		public:
			TestContext()
					: m_createCompletedRound(false)
					, m_hashes(test::GenerateRandomDataVector<Hash256>(3)) {
				// note: current voting round (8) is ahead of last round resulting in finalization (5)

				// set up filesystem
				const auto& userConfig = testState().state().config().User;
				const_cast<std::string&>(userConfig.DataDirectory) = m_directoryGuard.name();
				SeedOtsTree(userConfig);
				SeedVotingStatus(userConfig, FinalizationPoint(8));

				// register hooks
				auto pHooks = std::make_shared<FinalizationServerHooks>();
				pHooks->setMessageRangeConsumer([&messages = m_messages](auto&& messageRange) {
					auto extractedMessages = model::FinalizationMessageRange::ExtractEntitiesFromRange(std::move(messageRange.Range));
					messages.insert(messages.end(), extractedMessages.cbegin(), extractedMessages.cend());
				});
				locator().registerRootedService("fin.hooks", pHooks);

				// register storage
				auto lastFinalizedHeightHashPair = model::HeightHashPair{ Height(244), m_hashes[0] };
				auto pProofStorage = std::make_unique<mocks::MockProofStorage>(
						Finalization_Epoch,
						FinalizationPoint(5),
						lastFinalizedHeightHashPair.Height,
						lastFinalizedHeightHashPair.Hash);
				m_pProofStorageRaw = pProofStorage.get();
				locator().registerRootedService("fin.proof.storage", std::make_shared<io::ProofStorageCache>(std::move(pProofStorage)));

				// register aggregator
				m_pAggregator = std::make_shared<chain::MultiRoundMessageAggregator>(
						10'000'000,
						model::FinalizationRound{ Finalization_Epoch, FinalizationPoint(5) },
						lastFinalizedHeightHashPair,
						[this](const auto& round) {
							auto pRoundMessageAggregator = std::make_unique<mocks::MockRoundMessageAggregator>(round);
							if (m_createCompletedRound) {
								pRoundMessageAggregator->roundContext().acceptPrevote(Height(244), m_hashes.data(), m_hashes.size(), 750);
								pRoundMessageAggregator->roundContext().acceptPrecommit(Height(245), m_hashes[1], 400);
								pRoundMessageAggregator->roundContext().acceptPrecommit(Height(246), m_hashes[2], 400);
							}

							auto pMessage = test::CreateMessage(round);
							pMessage->Height = Height(245);

							chain::RoundMessageAggregator::UnknownMessages messages;
							messages.push_back(std::move(pMessage));
							pRoundMessageAggregator->setMessages(std::move(messages));
							return pRoundMessageAggregator;
						});
				locator().registerRootedService("fin.aggregator.multiround", m_pAggregator);
			}

			~TestContext() {
				// destroy service, which holds open voting_ots_tree.dat file handle, before removing temp directory
				destroy();
			}

		public:
			const auto& hashes() const {
				return m_hashes;
			}

			const auto& proofStorage() const {
				return *m_pProofStorageRaw;
			}

			const auto& aggregator() const {
				return *m_pAggregator;
			}

			const auto& messages() const {
				return m_messages;
			}

			auto votingStatus() {
				const auto& userConfig = testState().state().config().User;
				auto votingStatusFilename = config::CatapultDataDirectory(userConfig.DataDirectory).rootDir().file("voting_status.dat");
				return VotingStatusFile(votingStatusFilename).load();
			}

		public:
			void createCompletedRound() {
				m_createCompletedRound = true;
			}

			void setHash(size_t index, const Hash256& hash) {
				m_hashes[index] = hash;
			}

			void initialize() {
				auto votingRound = model::FinalizationRound{ Finalization_Epoch, FinalizationPoint(8) };
				auto aggregatorModifier = m_pAggregator->modifier();
				aggregatorModifier.setMaxFinalizationRound(votingRound);
				aggregatorModifier.add(test::CreateMessage(votingRound));
			}

		private:
			static auto CreateOtsKeyIdentifier(FinalizationPoint point, model::FinalizationStage stage) {
				return model::StepIdentifierToOtsKeyIdentifier({ FinalizationEpoch(), point, stage }, Ots_Key_Dilution);
			}

			static void SeedOtsTree(const config::UserConfiguration& userConfig) {
				auto votingOtsTreeFilename = config::CatapultDataDirectory(userConfig.DataDirectory).rootDir().file("voting_ots_tree.dat");
				io::FileStream otsStream(io::RawFile(votingOtsTreeFilename, io::OpenMode::Read_Write));

				auto startKeyIdentifier = CreateOtsKeyIdentifier(FinalizationPoint(1), Prevote_Stage);
				auto endKeyIdentifier = CreateOtsKeyIdentifier(FinalizationPoint(100), Precommit_Stage);
				crypto::OtsTree::Create(test::GenerateKeyPair(), otsStream, { Ots_Key_Dilution, startKeyIdentifier, endKeyIdentifier });
			}

			static void SeedVotingStatus(const config::UserConfiguration& userConfig, FinalizationPoint point) {
				auto votingStatusFilename = config::CatapultDataDirectory(userConfig.DataDirectory).rootDir().file("voting_status.dat");
				VotingStatusFile(votingStatusFilename).save({ { Finalization_Epoch, point }, false, false });
			}

		private:
			bool m_createCompletedRound;
			std::vector<Hash256> m_hashes;

			mocks::MockProofStorage* m_pProofStorageRaw;
			std::shared_ptr<chain::MultiRoundMessageAggregator> m_pAggregator;
			std::vector<std::shared_ptr<model::FinalizationMessage>> m_messages;

			test::TempDirectoryGuard m_directoryGuard;
		};
	}

	// endregion

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(FinalizationOrchestrator, Post_Extended_Range_Consumers)

	TEST(TEST_CLASS, OrchestratorServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(1u + Num_Dependent_Services, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());

		// - service (get does not throw)
		context.locator().service<void>("fin.orchestrator");
	}

	TEST(TEST_CLASS, TasksAreRegistered) {
		test::AssertRegisteredTasks(TestContext(), { "finalization task" });
	}

	// endregion

	// region task

	namespace {
		template<typename TCheckState>
		void RunFinalizationTaskTest(TestContext& context, size_t numRepetitions, uint64_t votingSetGrouping, TCheckState checkState) {
			// Arrange:
			context.initialize();
			context.boot(votingSetGrouping);

			test::RunTaskTestPostBoot(context, 1, "finalization task", [&context, numRepetitions, checkState](const auto& task) {
				// Act: run task multiple times
				std::vector<thread::TaskResult> taskResults;
				for (auto i = 0u; i < numRepetitions; ++i)
					taskResults.push_back(task.Callback().get());

				// Assert:
				for (auto i = 0u; i < numRepetitions; ++i)
					EXPECT_EQ(thread::TaskResult::Continue, taskResults[i]) << "result at " << i;

				checkState(context.aggregator(), context.testState().finalizationSubscriber(), context.proofStorage(), context.messages());
			});
		}
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereAreNoPendingFinalizedBlocks) {
		// Arrange:
		TestContext context;

		RunFinalizationTaskTest(context, 5, Default_Voting_Set_Grouping, [&context](
				const auto& aggregator,
				const auto& subscriber,
				const auto& storage,
				const auto& messages) {
			// Assert: check aggregator
			auto epoch = Finalization_Epoch.unwrap();
			EXPECT_EQ(test::CreateFinalizationRound(epoch, 5), aggregator.view().minFinalizationRound());
			EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), aggregator.view().maxFinalizationRound());

			// - subscriber and storage weren't called
			EXPECT_TRUE(subscriber.finalizedBlockParams().params().empty());
			EXPECT_TRUE(storage.savedProofDescriptors().empty());

			// - no messages were sent
			EXPECT_TRUE(messages.empty());

			// - voting status wasn't changed
			auto votingStatus = context.votingStatus();
			EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), votingStatus.Round);
			EXPECT_FALSE(votingStatus.HasSentPrevote);
			EXPECT_FALSE(votingStatus.HasSentPrecommit);
		});
	}

	namespace {
		void AssertCanRunFinalizationTaskWhenThereArePendingFinalizedBlocks(
				size_t numRepetitions,
				FinalizationPoint expectedMaxFinalizationPoint) {
			// Arrange:
			TestContext context;
			context.createCompletedRound();

			RunFinalizationTaskTest(context, numRepetitions, Default_Voting_Set_Grouping, [&context, expectedMaxFinalizationPoint](
					const auto& aggregator,
					const auto& subscriber,
					const auto& storage,
					const auto& messages) {
				const auto& expectedHash = context.hashes()[1];

				// Assert: check aggregator
				auto epoch = Finalization_Epoch.unwrap();
				auto expectedMaxRound = test::CreateFinalizationRound(epoch, expectedMaxFinalizationPoint.unwrap());
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 5), aggregator.view().minFinalizationRound());
				EXPECT_EQ(expectedMaxRound, aggregator.view().maxFinalizationRound());

				// - subscriber was called
				const auto& subscriberParams = subscriber.finalizedBlockParams().params();
				ASSERT_EQ(1u, subscriberParams.size());
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), subscriberParams[0].Round);
				EXPECT_EQ(Height(245), subscriberParams[0].Height);
				EXPECT_EQ(expectedHash, subscriberParams[0].Hash);

				// - storage was called (proof step identifier comes from test::CreateMessage)
				const auto& savedProofDescriptors = storage.savedProofDescriptors();
				ASSERT_EQ(1u, savedProofDescriptors.size());
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), savedProofDescriptors[0].Round);
				EXPECT_EQ(Height(245), savedProofDescriptors[0].Height);
				EXPECT_EQ(expectedHash, savedProofDescriptors[0].Hash);

				// - two messages were sent
				ASSERT_EQ(2u, messages.size());
				EXPECT_EQ(test::CreateStepIdentifier(epoch, 8, Prevote_Stage), messages[0]->StepIdentifier);
				EXPECT_EQ(test::CreateStepIdentifier(epoch, 8, Precommit_Stage), messages[1]->StepIdentifier);

				// - voting status was changed
				auto votingStatus = context.votingStatus();
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 9), votingStatus.Round);
				EXPECT_FALSE(votingStatus.HasSentPrevote);
				EXPECT_FALSE(votingStatus.HasSentPrecommit);
			});
		}
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereArePendingFinalizedBlocks_OnePoll) {
		AssertCanRunFinalizationTaskWhenThereArePendingFinalizedBlocks(1, FinalizationPoint(8));
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereArePendingFinalizedBlocks_MultiplePolls) {
		// Assert: maxFinalizationPoint is updated at beginning of second task execution
		AssertCanRunFinalizationTaskWhenThereArePendingFinalizedBlocks(5, FinalizationPoint(9));
	}

	namespace {
		void AssertCanRunFinalizationTaskWhenThereIsPendingInconsistentFinalizedEpoch(uint32_t numBlocks) {
			// Arrange:
			TestContext context;
			context.createCompletedRound();
			mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), numBlocks);

			RunFinalizationTaskTest(context, 2, 49, [&context](
					const auto& aggregator,
					const auto& subscriber,
					const auto& storage,
					const auto& messages) {
				const auto& expectedHash = context.hashes()[1];

				// - check aggregator (it did not advance the epoch)
				auto epoch = Finalization_Epoch.unwrap();
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 5), aggregator.view().minFinalizationRound());
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), aggregator.view().maxFinalizationRound());

				// - subscriber was called
				const auto& subscriberParams = subscriber.finalizedBlockParams().params();
				ASSERT_EQ(1u, subscriberParams.size());
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), subscriberParams[0].Round);
				EXPECT_EQ(Height(245), subscriberParams[0].Height);
				EXPECT_EQ(expectedHash, subscriberParams[0].Hash);

				// - storage was called (proof step identifier comes from test::CreateMessage)
				const auto& savedProofDescriptors = storage.savedProofDescriptors();
				ASSERT_EQ(1u, savedProofDescriptors.size());
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), savedProofDescriptors[0].Round);
				EXPECT_EQ(Height(245), savedProofDescriptors[0].Height);
				EXPECT_EQ(expectedHash, savedProofDescriptors[0].Hash);

				// - two messages were sent
				ASSERT_EQ(2u, messages.size());
				EXPECT_EQ(test::CreateStepIdentifier(epoch, 8, Prevote_Stage), messages[0]->StepIdentifier);
				EXPECT_EQ(test::CreateStepIdentifier(epoch, 8, Precommit_Stage), messages[1]->StepIdentifier);

				// - voting status was changed
				auto votingStatus = context.votingStatus();
				EXPECT_EQ(test::CreateFinalizationRound(epoch, 9), votingStatus.Round);
				EXPECT_FALSE(votingStatus.HasSentPrevote);
				EXPECT_FALSE(votingStatus.HasSentPrecommit);
			});
		}
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereIsPendingInconsistentFinalizedEpoch_InsufficientChainHeight) {
		AssertCanRunFinalizationTaskWhenThereIsPendingInconsistentFinalizedEpoch(170);
		AssertCanRunFinalizationTaskWhenThereIsPendingInconsistentFinalizedEpoch(244);
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereIsPendingInconsistentFinalizedEpoch_IncorrectHashInStorage) {
		AssertCanRunFinalizationTaskWhenThereIsPendingInconsistentFinalizedEpoch(245);
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereIsPendingFinalizedEpoch) {
		// Arrange:
		TestContext context;
		context.createCompletedRound();

		// - override the storage hash so that it matches
		auto& blockStorage = context.testState().state().storage();
		mocks::SeedStorageWithFixedSizeBlocks(blockStorage, 245);
		context.setHash(1, blockStorage.view().loadBlockElement(Height(245))->EntityHash);

		RunFinalizationTaskTest(context, 2, 49, [&context](
				const auto& aggregator,
				const auto& subscriber,
				const auto& storage,
				const auto& messages) {
			const auto& expectedHash = context.hashes()[1];

			// - check aggregator (it advanced the epoch)
			auto epoch = Finalization_Epoch.unwrap();
			EXPECT_EQ(test::CreateFinalizationRound(epoch, 5), aggregator.view().minFinalizationRound());
			EXPECT_EQ(test::CreateFinalizationRound(epoch + 1, 1), aggregator.view().maxFinalizationRound());

			// - subscriber was called
			const auto& subscriberParams = subscriber.finalizedBlockParams().params();
			ASSERT_EQ(1u, subscriberParams.size());
			EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), subscriberParams[0].Round);
			EXPECT_EQ(Height(245), subscriberParams[0].Height);
			EXPECT_EQ(expectedHash, subscriberParams[0].Hash);

			// - storage was called (proof step identifier comes from test::CreateMessage)
			const auto& savedProofDescriptors = storage.savedProofDescriptors();
			ASSERT_EQ(1u, savedProofDescriptors.size());
			EXPECT_EQ(test::CreateFinalizationRound(epoch, 8), savedProofDescriptors[0].Round);
			EXPECT_EQ(Height(245), savedProofDescriptors[0].Height);
			EXPECT_EQ(expectedHash, savedProofDescriptors[0].Hash);

			// - two messages were sent
			ASSERT_EQ(2u, messages.size());
			EXPECT_EQ(test::CreateStepIdentifier(epoch, 8, Prevote_Stage), messages[0]->StepIdentifier);
			EXPECT_EQ(test::CreateStepIdentifier(epoch, 8, Precommit_Stage), messages[1]->StepIdentifier);

			// - voting status was changed
			auto votingStatus = context.votingStatus();
			EXPECT_EQ(test::CreateFinalizationRound(epoch + 1, 1), votingStatus.Round);
			EXPECT_FALSE(votingStatus.HasSentPrevote);
			EXPECT_FALSE(votingStatus.HasSentPrecommit);
		});
	}

	// endregion
}}
