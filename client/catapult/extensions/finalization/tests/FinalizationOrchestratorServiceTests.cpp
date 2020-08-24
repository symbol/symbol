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

		struct FinalizationOrchestratorServiceTraits {
			static auto CreateRegistrar() {
				// (Size, Threshold) are set in MockRoundMessageAggregator to (1000, 750)
				auto config = FinalizationConfiguration::Uninitialized();
				config.StepDuration = utils::TimeSpan::FromSeconds(10);
				config.MaxHashesPerPoint = 64;
				config.PrevoteBlocksMultiple = 5;
				config.VotingSetGrouping = 500;
				return CreateFinalizationOrchestratorServiceRegistrar(config);
			}
		};

		class TestContext : public test::ServiceLocatorTestContext<FinalizationOrchestratorServiceTraits> {
		private:
			using BaseType = test::ServiceLocatorTestContext<FinalizationOrchestratorServiceTraits>;

		public:
			TestContext()
					: m_createCompletedRound(false)
					, m_hashes(test::GenerateRandomDataVector<Hash256>(3)) {
				// set up filesystem
				SeedOtsTree(testState().state().config().User, m_directoryGuard.name());

				// register hooks
				auto pHooks = std::make_shared<FinalizationServerHooks>();
				pHooks->setMessageRangeConsumer([&messages = m_messages](auto&& messageRange) {
					auto extractedMessages = model::FinalizationMessageRange::ExtractEntitiesFromRange(std::move(messageRange.Range));
					messages.insert(messages.end(), extractedMessages.cbegin(), extractedMessages.cend());
				});
				locator().registerRootedService("fin.hooks", pHooks);

				// register storage
				auto lastFinalizedHeightHashPair = model::HeightHashPair{ Height(245), m_hashes[0] };
				auto pProofStorage = std::make_unique<mocks::MockProofStorage>(
						FinalizationPoint(7),
						lastFinalizedHeightHashPair.Height,
						lastFinalizedHeightHashPair.Hash);
				m_pProofStorageRaw = pProofStorage.get();
				locator().registerRootedService("fin.proof.storage", std::make_shared<io::ProofStorageCache>(std::move(pProofStorage)));

				// register aggregator
				m_pAggregator = std::make_shared<chain::MultiRoundMessageAggregator>(
						10'000'000,
						FinalizationPoint(8),
						lastFinalizedHeightHashPair,
						[this](auto roundPoint, auto height) {
							auto pRoundMessageAggregator = std::make_unique<mocks::MockRoundMessageAggregator>(roundPoint, height);
							if (m_createCompletedRound) {
								pRoundMessageAggregator->roundContext().acceptPrevote(Height(245), m_hashes.data(), m_hashes.size(), 750);
								pRoundMessageAggregator->roundContext().acceptPrecommit(Height(246), m_hashes[1], 400);
								pRoundMessageAggregator->roundContext().acceptPrecommit(Height(247), m_hashes[2], 400);
							}

							auto pMessage = test::CreateMessage(roundPoint);
							pMessage->Height = Height(246);

							chain::RoundMessageAggregator::UnknownMessages messages;
							messages.push_back(std::move(pMessage));
							pRoundMessageAggregator->setMessages(std::move(messages));
							return pRoundMessageAggregator;
						});
				locator().registerRootedService("fin.aggregator.multiround", m_pAggregator);
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

		public:
			void createCompletedRound() {
				m_createCompletedRound = true;
			}

			void initialize() {
				m_pAggregator->modifier().add(test::CreateMessage(FinalizationPoint(8)));
			}

		private:
			static void SeedOtsTree(const config::UserConfiguration& userConfig, const std::string& dataDirectory) {
				const_cast<std::string&>(userConfig.DataDirectory) = dataDirectory;

				auto votingOtsTreeFilename = config::CatapultDataDirectory(userConfig.DataDirectory).rootDir().file("voting_ots_tree.dat");
				io::FileStream otsStream(io::RawFile(votingOtsTreeFilename, io::OpenMode::Read_Write));

				auto dilution = 13u;
				auto startKeyIdentifier = model::StepIdentifierToOtsKeyIdentifier(
						{ FinalizationPoint(1), model::FinalizationStage::Prevote },
						dilution);
				auto endKeyIdentifier = model::StepIdentifierToOtsKeyIdentifier(
						{ FinalizationPoint(100), model::FinalizationStage::Precommit },
						dilution);
				crypto::OtsTree::Create(test::GenerateKeyPair(), otsStream, { dilution, startKeyIdentifier, endKeyIdentifier });
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

	// endregion

	// region task

	namespace {
		template<typename TCheckState>
		void RunFinalizationTaskTest(TestContext& context, TCheckState checkState) {
			// Arrange:
			mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), 25);
			context.initialize();
			context.boot();

			test::RunTaskTestPostBoot(context, 1, "finalization task", [&context, checkState](const auto& task) {
				// Act: run task multiple times because results should be idempotent
				auto result1 = task.Callback().get();
				auto result2 = task.Callback().get();

				// Assert:
				EXPECT_EQ(thread::TaskResult::Continue, result1);
				EXPECT_EQ(thread::TaskResult::Continue, result2);

				checkState(context.aggregator(), context.testState().finalizationSubscriber(), context.proofStorage(), context.messages());
			});
		}
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereAreNoPendingFinalizedBlocks) {
		// Arrange:
		TestContext context;

		RunFinalizationTaskTest(context, [](const auto& aggregator, const auto& subscriber, const auto& storage, const auto& messages) {
			// Assert: check aggregator
			EXPECT_EQ(FinalizationPoint(8), aggregator.view().minFinalizationPoint());
			EXPECT_EQ(FinalizationPoint(8), aggregator.view().maxFinalizationPoint());

			// - subscriber and storage weren't called
			EXPECT_TRUE(subscriber.finalizedBlockParams().params().empty());
			EXPECT_TRUE(storage.savedProofDescriptors().empty());

			// - no messages were sent
			EXPECT_TRUE(messages.empty());
		});
	}

	TEST(TEST_CLASS, CanRunFinalizationTaskWhenThereArePendingFinalizedBlocks) {
		// Arrange:
		TestContext context;
		context.createCompletedRound();

		const auto& expectedHash = context.hashes()[1];
		RunFinalizationTaskTest(context, [expectedHash](
				const auto& aggregator,
				const auto& subscriber,
				const auto& storage,
				const auto& messages) {
			// Assert: check aggregator
			EXPECT_EQ(FinalizationPoint(8), aggregator.view().minFinalizationPoint());
			EXPECT_EQ(FinalizationPoint(9), aggregator.view().maxFinalizationPoint());

			// - subscriber was called
			const auto& subscriberParams = subscriber.finalizedBlockParams().params();
			ASSERT_EQ(1u, subscriberParams.size());
			EXPECT_EQ(Height(246), subscriberParams[0].Height);
			EXPECT_EQ(expectedHash, subscriberParams[0].Hash);
			EXPECT_EQ(FinalizationPoint(8), subscriberParams[0].Point);

			// - storage was called (proof step identifier comes from test::CreateMessage)
			const auto& savedProofDescriptors = storage.savedProofDescriptors();
			ASSERT_EQ(1u, savedProofDescriptors.size());
			EXPECT_EQ(Height(246), savedProofDescriptors[0].Height);
			EXPECT_EQ(expectedHash, savedProofDescriptors[0].Hash);
			EXPECT_EQ(FinalizationPoint(8), savedProofDescriptors[0].Point);

			// - two messages were sent
			ASSERT_EQ(2u, messages.size());
			EXPECT_EQ(model::StepIdentifier({ FinalizationPoint(8), model::FinalizationStage::Prevote }), messages[0]->StepIdentifier);
			EXPECT_EQ(model::StepIdentifier({ FinalizationPoint(8), model::FinalizationStage::Precommit }), messages[1]->StepIdentifier);
		});
	}

	// endregion
}}
