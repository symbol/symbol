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

#include "finalization/src/chain/FinalizationOrchestrator.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "finalization/tests/test/mocks/MockRoundMessageAggregator.h"
#include "tests/test/other/mocks/MockFinalizationSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationOrchestratorTests

	namespace {
		// region MockFinalizationMessageFactory

		enum class MessageType { Prevote, Precommit };

		class MockFinalizationMessageFactory : public FinalizationMessageFactory {
		public:
			std::vector<MessageType> messageTypes() const {
				return m_messageTypes;
			}

		public:
			std::unique_ptr<model::FinalizationMessage> createPrevote(const model::FinalizationRound& round) override {
				m_messageTypes.push_back(MessageType::Prevote);
				auto pMessage = test::CreateMessage(Height(0), test::GenerateRandomByteArray<Hash256>());
				pMessage->StepIdentifier.Epoch = round.Epoch;
				pMessage->StepIdentifier.Point = round.Point;
				return pMessage;
			}

		public:
			std::unique_ptr<model::FinalizationMessage> createPrecommit(
					const model::FinalizationRound& round,
					Height height,
					const Hash256& hash) override {
				m_messageTypes.push_back(MessageType::Precommit);
				auto pMessage = test::CreateMessage(height, hash);
				pMessage->StepIdentifier.Epoch = round.Epoch;
				pMessage->StepIdentifier.Point = round.Point;
				return pMessage;
			}

		private:
			std::vector<MessageType> m_messageTypes;
		};

		void AssertPrevote(const model::FinalizationMessage& message, const model::FinalizationRound& round) {
			EXPECT_EQ(round.Epoch, message.StepIdentifier.Epoch);
			EXPECT_EQ(round.Point, message.StepIdentifier.Point);
			EXPECT_EQ(Height(0), message.Height);
		}

		void AssertPrecommit(
				const model::FinalizationMessage& message,
				const model::FinalizationRound& round,
				Height height,
				const Hash256& hash) {
			EXPECT_EQ(round.Epoch, message.StepIdentifier.Epoch);
			EXPECT_EQ(round.Point, message.StepIdentifier.Point);
			EXPECT_EQ(height, message.Height);
			EXPECT_EQ(hash, *message.HashesPtr());
		}

		// endregion

		// region MockFinalizationStageAdvancer

		class MockFinalizationStageAdvancer : public FinalizationStageAdvancer {
		public:
			MockFinalizationStageAdvancer(const model::FinalizationRound& round, Timestamp time)
					: m_round(round)
					, m_time(time)
					, m_canSendPrevote(false)
					, m_canSendPrecommit(false)
					, m_canStartNextRound(false)
			{}

		public:
			model::FinalizationRound round() const {
				return m_round;
			}

			Timestamp time() const {
				return m_time;
			}

			const auto& times() const {
				return m_times;
			}

		public:
			void setReturnValues(bool canSendPrevote, bool canSendPrecommit, bool canStartNextRound) {
				m_canSendPrevote = canSendPrevote;
				m_canSendPrecommit = canSendPrecommit;
				m_canStartNextRound = canStartNextRound;
			}

			void setTarget(const model::HeightHashPair& target) {
				m_target = target;
			}

		public:
			bool canSendPrevote(Timestamp time) const override {
				m_times.push_back(time);
				return m_canSendPrevote;
			}

			bool canSendPrecommit(Timestamp time, model::HeightHashPair& target) const override {
				target = m_target;
				m_times.push_back(time);
				return m_canSendPrecommit;
			}

			bool canStartNextRound() const override {
				return m_canStartNextRound;
			}

		private:
			model::FinalizationRound m_round;
			Timestamp m_time;

			bool m_canSendPrevote;
			bool m_canSendPrecommit;
			bool m_canStartNextRound;
			model::HeightHashPair m_target;

			mutable std::vector<Timestamp> m_times;
		};

		// endregion

		// region TestContext

		class TestContext {
		public:
			explicit TestContext(const model::FinalizationRound& round) : TestContext({ round, false, false })
			{}

			explicit TestContext(const VotingStatus& votingStatus)
					: m_pMessageFactory(std::make_unique<MockFinalizationMessageFactory>())
					, m_pMessageFactoryRaw(m_pMessageFactory.get())
					, m_orchestrator(
							votingStatus,
							[this](auto stageAdvancerPoint, auto time) {
								auto pStageAdvancer = std::make_unique<MockFinalizationStageAdvancer>(stageAdvancerPoint, time);
								m_stageAdvancers.push_back(pStageAdvancer.get());

								if (m_createCompletedStageAdvancer) {
									pStageAdvancer->setReturnValues(true, true, true);
									pStageAdvancer->setTarget(m_defaultTarget);
								}

								return pStageAdvancer;
							},
							[this](auto&& pMessage) {
								m_messages.push_back(std::move(pMessage));
							},
							std::move(m_pMessageFactory))
					, m_createCompletedStageAdvancer(false)
			{}

		public:
			auto& orchestrator() {
				return m_orchestrator;
			}

			const auto& orchestrator() const {
				return m_orchestrator;
			}

			const auto& messageFactory() const {
				return *m_pMessageFactoryRaw;
			}

			const auto& messages() const {
				return m_messages;
			}

			auto& stageAdvancers() const {
				return m_stageAdvancers;
			}

		public:
			void createCompletedStageAdvancer(const model::HeightHashPair& target) {
				m_createCompletedStageAdvancer = true;
				m_defaultTarget = target;
			}

		private:
			std::unique_ptr<MockFinalizationMessageFactory> m_pMessageFactory; // moved into m_orchestrator
			MockFinalizationMessageFactory* m_pMessageFactoryRaw;
			FinalizationOrchestrator m_orchestrator;

			bool m_createCompletedStageAdvancer;
			model::HeightHashPair m_defaultTarget;
			std::vector<MockFinalizationStageAdvancer*> m_stageAdvancers;
			std::vector<std::unique_ptr<model::FinalizationMessage>> m_messages;
		};

		// endregion

		// region test utils

		auto ToTimestamps(std::initializer_list<Timestamp::ValueType> values) {
			std::vector<Timestamp> timestamps;
			for (auto value : values)
				timestamps.push_back(Timestamp(value));

			return timestamps;
		}

		void AssertVotingStatus(const VotingStatus& expected, const VotingStatus& actual) {
			EXPECT_EQ(expected.Round, actual.Round);
			EXPECT_EQ(expected.HasSentPrevote, actual.HasSentPrevote);
			EXPECT_EQ(expected.HasSentPrecommit, actual.HasSentPrecommit);
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateOrchestrator) {
		// Act:
		TestContext context(test::CreateFinalizationRound(3, 4));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 4), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>(), context.messageFactory().messageTypes());
		EXPECT_EQ(0u, context.messages().size());
		EXPECT_EQ(0u, context.stageAdvancers().size());
	}

	// endregion

	// region setEpoch

	namespace {
		template<typename TAction>
		void RunSetEpochTest(TAction action) {
			// Arrange:
			TestContext context({ test::CreateFinalizationRound(11, 4), true, true });

			// Sanity:
			AssertVotingStatus({ test::CreateFinalizationRound(11, 4), true, true }, context.orchestrator().votingStatus());

			// Act + Assert:
			action(context);
		}
	}

	TEST(TEST_CLASS, SetEpochCannotDecreaseCurrentEpoch) {
		// Arrange:
		RunSetEpochTest([](auto& context) {
			// Act + Assert:
			EXPECT_THROW(context.orchestrator().setEpoch(FinalizationEpoch(10)), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, SetEpochHasNoEffectWhenCurrentEpochIsEqual) {
		// Arrange:
		RunSetEpochTest([](auto& context) {
			// Act:
			context.orchestrator().setEpoch(FinalizationEpoch(11));

			// Assert:
			AssertVotingStatus({ test::CreateFinalizationRound(11, 4), true, true }, context.orchestrator().votingStatus());
		});
	}

	TEST(TEST_CLASS, SetEpochCanIncreaseCurrentEpoch) {
		// Arrange:
		RunSetEpochTest([](auto& context) {
			// Act:
			context.orchestrator().setEpoch(FinalizationEpoch(15));

			// Assert:
			AssertVotingStatus({ test::CreateFinalizationRound(15, 1), false, false }, context.orchestrator().votingStatus());
		});
	}

	TEST(TEST_CLASS, PollCreatesNewAdvancerAfterSetEpochAdvancesEpoch) {
		// Arrange:
		TestContext context({ test::CreateFinalizationRound(3, 4), true, true });
		context.orchestrator().poll(Timestamp(100));
		context.orchestrator().setEpoch(FinalizationEpoch(11));

		// Act:
		context.orchestrator().poll(Timestamp(200));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(11, 1), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>(), context.messageFactory().messageTypes());
		EXPECT_EQ(0u, context.messages().size());

		ASSERT_EQ(2u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(11, 1), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(200), context.stageAdvancers().back()->time());
		EXPECT_EQ(ToTimestamps({ 200, 200 }), context.stageAdvancers().back()->times());
	}

	// endregion

	// region poll - start round

	TEST(TEST_CLASS, PollCreatesStageAdvancerForCurrentRoundWhenNoAdvancerIsPresent) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));

		// Act:
		context.orchestrator().poll(Timestamp(100));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 4), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>(), context.messageFactory().messageTypes());
		EXPECT_EQ(0u, context.messages().size());

		ASSERT_EQ(1u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(3, 4), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
		EXPECT_EQ(ToTimestamps({ 100, 100 }), context.stageAdvancers().back()->times());
	}

	TEST(TEST_CLASS, PollDoesNotCreateStageAdvancerForNextRoundWhenAdvancerCanStartNextRoundButPrecommitHasNotBeenSent) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));
		context.orchestrator().poll(Timestamp(100));

		context.stageAdvancers()[0]->setReturnValues(true, false, true);

		// Act:
		context.orchestrator().poll(Timestamp(200));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 4), true, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>({ MessageType::Prevote }), context.messageFactory().messageTypes());
		EXPECT_EQ(1u, context.messages().size());

		ASSERT_EQ(1u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(3, 4), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
		EXPECT_EQ(ToTimestamps({ 100, 100, 200, 200 }), context.stageAdvancers().back()->times());
	}

	TEST(TEST_CLASS, PollCreatesStageAdvancerForNextRoundWhenAdvancerCanStartNextRound) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));
		context.orchestrator().poll(Timestamp(100));

		context.stageAdvancers()[0]->setReturnValues(true, true, true);

		// Act:
		context.orchestrator().poll(Timestamp(200));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 5), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>({ MessageType::Prevote, MessageType::Precommit }), context.messageFactory().messageTypes());
		EXPECT_EQ(2u, context.messages().size());

		ASSERT_EQ(2u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(3, 5), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(200), context.stageAdvancers().back()->time());
		EXPECT_EQ(std::vector<Timestamp>(), context.stageAdvancers().back()->times());
	}

	// endregion

	// region poll - send prevote

	namespace {
		void AssertSinglePrevoteSent(const TestContext& context, const std::vector<Timestamp>& expectedStageAdvancerTimes) {
			// Assert:
			AssertVotingStatus({ test::CreateFinalizationRound(3, 4), true, false }, context.orchestrator().votingStatus());

			EXPECT_EQ(std::vector<MessageType>({ MessageType::Prevote }), context.messageFactory().messageTypes());
			ASSERT_EQ(1u, context.messages().size());
			AssertPrevote(*context.messages()[0], { FinalizationEpoch(3), FinalizationPoint(4) });

			ASSERT_EQ(1u, context.stageAdvancers().size());
			EXPECT_EQ(test::CreateFinalizationRound(3, 4), context.stageAdvancers().back()->round());
			EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
			EXPECT_EQ(expectedStageAdvancerTimes, context.stageAdvancers().back()->times());
		}
	}

	TEST(TEST_CLASS, PollSendsPrevoteWhenAdvancerCanSendPrevoteAndPrevoteNotAlreadySent) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));
		context.orchestrator().poll(Timestamp(100));

		context.stageAdvancers()[0]->setReturnValues(true, false, false);

		// Act:
		context.orchestrator().poll(Timestamp(200));

		// Assert:
		AssertSinglePrevoteSent(context, ToTimestamps({ 100, 100, 200, 200 }));
	}

	TEST(TEST_CLASS, PollDoesNotSendPrevoteWhenAdvancerCanSendPrevoteAndPrevoteAlreadySent) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));
		context.orchestrator().poll(Timestamp(100));

		context.stageAdvancers()[0]->setReturnValues(true, false, false);

		context.orchestrator().poll(Timestamp(200));

		// Act:
		context.orchestrator().poll(Timestamp(300));

		// Assert:
		AssertSinglePrevoteSent(context, ToTimestamps({ 100, 100, 200, 200, 300 }));
	}

	// endregion

	// region poll - send precommit

	namespace {
		void AssertSinglePrecommitSent(const TestContext& context, const Hash256& precommitHash) {
			// Assert:
			AssertVotingStatus({ test::CreateFinalizationRound(3, 4), true, true }, context.orchestrator().votingStatus());

			EXPECT_EQ(std::vector<MessageType>({ MessageType::Prevote, MessageType::Precommit }), context.messageFactory().messageTypes());
			ASSERT_EQ(2u, context.messages().size());
			AssertPrevote(*context.messages()[0], { FinalizationEpoch(3), FinalizationPoint(4) });
			AssertPrecommit(*context.messages()[1], { FinalizationEpoch(3), FinalizationPoint(4) }, Height(123), precommitHash);

			ASSERT_EQ(1u, context.stageAdvancers().size());
			EXPECT_EQ(test::CreateFinalizationRound(3, 4), context.stageAdvancers().back()->round());
			EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
			EXPECT_EQ(ToTimestamps({ 100, 100, 200, 200 }), context.stageAdvancers().back()->times());
		}
	}

	TEST(TEST_CLASS, PollSendsPrecommitWhenAdvancerCanSendPrecommitAndPrecommitNotAlreadySent) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));
		context.orchestrator().poll(Timestamp(100));

		auto hash = test::GenerateRandomByteArray<Hash256>();
		context.stageAdvancers()[0]->setReturnValues(true, true, false);
		context.stageAdvancers()[0]->setTarget({ Height(123), hash });

		// Act:
		context.orchestrator().poll(Timestamp(200));

		// Assert:
		AssertSinglePrecommitSent(context, hash);
	}

	TEST(TEST_CLASS, PollDoesNotSendPrecommitWhenAdvancerCanSendPrecommitAndPrecommitAlreadySent) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));
		context.orchestrator().poll(Timestamp(100));

		auto hash = test::GenerateRandomByteArray<Hash256>();
		context.stageAdvancers()[0]->setReturnValues(true, true, false);
		context.stageAdvancers()[0]->setTarget({ Height(123), hash });

		context.orchestrator().poll(Timestamp(200));

		// Act:
		context.orchestrator().poll(Timestamp(300));

		// Assert:
		AssertSinglePrecommitSent(context, hash);
	}

	// endregion

	// region poll - batch

	TEST(TEST_CLASS, PollCanProgressThroughEntireRoundInOneCall) {
		// Arrange:
		TestContext context(test::CreateFinalizationRound(3, 4));

		auto hash = test::GenerateRandomByteArray<Hash256>();
		context.createCompletedStageAdvancer({ Height(123), hash });

		// Act:
		context.orchestrator().poll(Timestamp(100));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 5), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>({ MessageType::Prevote, MessageType::Precommit }), context.messageFactory().messageTypes());
		ASSERT_EQ(2u, context.messages().size());
		AssertPrevote(*context.messages()[0], { FinalizationEpoch(3), FinalizationPoint(4) });
		AssertPrecommit(*context.messages()[1], { FinalizationEpoch(3), FinalizationPoint(4) }, Height(123), hash);

		ASSERT_EQ(2u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(3, 5), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
		EXPECT_EQ(std::vector<Timestamp>(), context.stageAdvancers().back()->times());
	}

	TEST(TEST_CLASS, PollCanProgressThroughEntireRoundInOneCall_PreviouslySentPrevote) {
		// Arrange:
		TestContext context({ test::CreateFinalizationRound(3, 4), true, false });

		auto hash = test::GenerateRandomByteArray<Hash256>();
		context.createCompletedStageAdvancer({ Height(123), hash });

		// Act:
		context.orchestrator().poll(Timestamp(100));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 5), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>({ MessageType::Precommit }), context.messageFactory().messageTypes());
		ASSERT_EQ(1u, context.messages().size());
		AssertPrecommit(*context.messages()[0], { FinalizationEpoch(3), FinalizationPoint(4) }, Height(123), hash);

		ASSERT_EQ(2u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(3, 5), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
		EXPECT_EQ(std::vector<Timestamp>(), context.stageAdvancers().back()->times());
	}

	TEST(TEST_CLASS, PollCanProgressThroughEntireRoundInOneCall_PreviouslySentPrecommit) {
		// Arrange:
		TestContext context({ test::CreateFinalizationRound(3, 4), true, true });

		auto hash = test::GenerateRandomByteArray<Hash256>();
		context.createCompletedStageAdvancer({ Height(123), hash });

		// Act:
		context.orchestrator().poll(Timestamp(100));

		// Assert:
		AssertVotingStatus({ test::CreateFinalizationRound(3, 5), false, false }, context.orchestrator().votingStatus());

		EXPECT_EQ(std::vector<MessageType>(), context.messageFactory().messageTypes());
		ASSERT_EQ(0u, context.messages().size());

		ASSERT_EQ(2u, context.stageAdvancers().size());
		EXPECT_EQ(test::CreateFinalizationRound(3, 5), context.stageAdvancers().back()->round());
		EXPECT_EQ(Timestamp(100), context.stageAdvancers().back()->time());
		EXPECT_EQ(std::vector<Timestamp>(), context.stageAdvancers().back()->times());
	}

	// endregion

	namespace {
		constexpr auto Finalizer_Finalization_Epoch = FinalizationEpoch(11);

		// region CreateFinalizerTestContext

		class CreateFinalizerTestContext {
		private:
			using RoundMessageAggregatorInitializer = consumer<mocks::MockRoundMessageAggregator&>;

		public:
			explicit CreateFinalizerTestContext(FinalizationPoint point)
					: CreateFinalizerTestContext(point, std::make_unique<mocks::MockProofStorage>())
			{}

			CreateFinalizerTestContext(FinalizationPoint point, std::unique_ptr<mocks::MockProofStorage>&& pProofStorage)
					: m_pProofStorage(std::move(pProofStorage))
					, m_pProofStorageRaw(m_pProofStorage.get())
					, m_proofStorageCache(std::move(m_pProofStorage)) {
				m_pAggregator = std::make_unique<MultiRoundMessageAggregator>(
						10'000'000,
						model::FinalizationRound{ Finalizer_Finalization_Epoch, point },
						model::HeightHashPair(),
						[this](const auto& round) {
							auto pRoundMessageAggregator = std::make_unique<mocks::MockRoundMessageAggregator>(round);
							if (m_roundMessageAggregatorInitializer)
								m_roundMessageAggregatorInitializer(*pRoundMessageAggregator);

							return pRoundMessageAggregator;
						});
			}

		public:
			const auto& aggregator() const {
				return *m_pAggregator;
			}

			const auto& subscriber() const {
				return m_subscriber;
			}

			const auto& proofStorage() const {
				return *m_pProofStorageRaw;
			}

		public:
			void finalize() {
				CreateFinalizer(*m_pAggregator, m_subscriber, m_proofStorageCache)();
			}

		public:
			void setRoundMessageAggregatorInitializer(const RoundMessageAggregatorInitializer& roundMessageAggregatorInitializer) {
				m_roundMessageAggregatorInitializer = roundMessageAggregatorInitializer;
			}

			void addMessages(FinalizationPoint minPoint, FinalizationPoint maxPoint) {
				auto modifier = m_pAggregator->modifier();
				modifier.setMaxFinalizationRound({ Finalizer_Finalization_Epoch, maxPoint });

				for (auto point = minPoint; point <= maxPoint; point = point + FinalizationPoint(1))
					modifier.add(test::CreateMessage({ Finalizer_Finalization_Epoch, point }));
			}

		private:
			std::unique_ptr<mocks::MockProofStorage> m_pProofStorage; // moved into m_proofStorageCache
			mocks::MockProofStorage* m_pProofStorageRaw;
			io::ProofStorageCache m_proofStorageCache;

			RoundMessageAggregatorInitializer m_roundMessageAggregatorInitializer;
			std::unique_ptr<MultiRoundMessageAggregator> m_pAggregator;
			mocks::MockFinalizationSubscriber m_subscriber;
		};

		// endregion
	}

	// region CreateFinalizer

	namespace {
		void AssertNotFinalized(const CreateFinalizerTestContext& context) {
			// Assert: aggregator wasn't pruned
			EXPECT_EQ(4u, context.aggregator().view().size());

			// - subscriber and storage weren't called
			EXPECT_TRUE(context.subscriber().finalizedBlockParams().params().empty());
			EXPECT_TRUE(context.proofStorage().savedProofDescriptors().empty());
		}
	}

	TEST(TEST_CLASS, CreateFinalizer_HasNoEffectWhenNoBestPrecommit) {
		// Arrange:
		CreateFinalizerTestContext context(FinalizationPoint(7));
		context.setRoundMessageAggregatorInitializer([](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			roundMessageAggregator.roundContext().acceptPrevote(Height(246), &hash, 1, 750);
		});

		context.addMessages(FinalizationPoint(7), FinalizationPoint(10));

		// Sanity:
		EXPECT_EQ(4u, context.aggregator().view().size());

		// Act:
		context.finalize();

		// Assert:
		AssertNotFinalized(context);
	}

	TEST(TEST_CLASS, CreateFinalizer_HasNoEffectWhenBestPrecommitHeightMatchesProofStorageHeight) {
		// Arrange:
		CreateFinalizerTestContext context(
				FinalizationPoint(7),
				std::make_unique<mocks::MockProofStorage>(FinalizationPoint(12), Height(246)));
		context.setRoundMessageAggregatorInitializer([](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			roundMessageAggregator.roundContext().acceptPrevote(Height(246), &hash, 1, 750);
			roundMessageAggregator.roundContext().acceptPrecommit(Height(246), hash, 750);
		});

		context.addMessages(FinalizationPoint(7), FinalizationPoint(10));

		// Sanity:
		EXPECT_EQ(4u, context.aggregator().view().size());

		// Act:
		context.finalize();

		// Assert:
		AssertNotFinalized(context);
	}

	TEST(TEST_CLASS, CreateFinalizer_FinalizesBlockWhenCurrentRoundHasBestPrecommit) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(4);
		CreateFinalizerTestContext context(FinalizationPoint(7));
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			const auto& hash = hashes[roundMessageAggregator.round().Point.unwrap() - 7];
			roundMessageAggregator.roundContext().acceptPrevote(Height(246), &hash, 1, 750);
			roundMessageAggregator.roundContext().acceptPrecommit(Height(246), hash, 750);

			RoundMessageAggregator::UnknownMessages messages;
			messages.push_back(test::CreateMessage(roundMessageAggregator.round()));
			roundMessageAggregator.setMessages(std::move(messages));
		});

		context.addMessages(FinalizationPoint(7), FinalizationPoint(10));

		// Sanity:
		EXPECT_EQ(4u, context.aggregator().view().size());

		// Act:
		context.finalize();

		// Assert: aggregator was not pruned
		EXPECT_EQ(4u, context.aggregator().view().size());

		// - subscriber was called
		const auto& subscriberParams = context.subscriber().finalizedBlockParams().params();
		ASSERT_EQ(1u, subscriberParams.size());
		EXPECT_EQ(test::CreateFinalizationRound(Finalizer_Finalization_Epoch.unwrap(), 10), subscriberParams[0].Round);
		EXPECT_EQ(Height(246), subscriberParams[0].Height);
		EXPECT_EQ(hashes[3], subscriberParams[0].Hash);

		// - storage was called (proof step identifier comes from test::CreateMessage)
		const auto& savedProofDescriptors = context.proofStorage().savedProofDescriptors();
		ASSERT_EQ(1u, savedProofDescriptors.size());
		EXPECT_EQ(test::CreateFinalizationRound(Finalizer_Finalization_Epoch.unwrap(), 10), savedProofDescriptors[0].Round);
		EXPECT_EQ(Height(246), savedProofDescriptors[0].Height);
		EXPECT_EQ(hashes[3], savedProofDescriptors[0].Hash);
	}

	TEST(TEST_CLASS, CreateFinalizer_FinalizesBlockWhenPreviousRoundHasBestPrecommit) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(4);
		CreateFinalizerTestContext context(FinalizationPoint(7));
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			auto roundPoint = roundMessageAggregator.round().Point;
			const auto& hash = hashes[roundPoint.unwrap() - 7];
			roundMessageAggregator.roundContext().acceptPrevote(Height(246), &hash, 1, 750);

			if (FinalizationPoint(8) >= roundPoint)
				roundMessageAggregator.roundContext().acceptPrecommit(Height(246), hash, 750);

			RoundMessageAggregator::UnknownMessages messages;
			messages.push_back(test::CreateMessage(roundMessageAggregator.round()));
			roundMessageAggregator.setMessages(std::move(messages));
		});

		context.addMessages(FinalizationPoint(7), FinalizationPoint(10));

		// Sanity:
		EXPECT_EQ(4u, context.aggregator().view().size());

		// Act:
		context.finalize();

		// Assert: aggregator was not pruned
		EXPECT_EQ(4u, context.aggregator().view().size());

		// - subscriber was called
		const auto& subscriberParams = context.subscriber().finalizedBlockParams().params();
		ASSERT_EQ(1u, subscriberParams.size());
		EXPECT_EQ(test::CreateFinalizationRound(Finalizer_Finalization_Epoch.unwrap(), 8), subscriberParams[0].Round);
		EXPECT_EQ(Height(246), subscriberParams[0].Height);
		EXPECT_EQ(hashes[1], subscriberParams[0].Hash);

		// - storage was called (proof step identifier comes from test::CreateMessage)
		const auto& savedProofDescriptors = context.proofStorage().savedProofDescriptors();
		ASSERT_EQ(1u, savedProofDescriptors.size());
		EXPECT_EQ(test::CreateFinalizationRound(Finalizer_Finalization_Epoch.unwrap(), 8), savedProofDescriptors[0].Round);
		EXPECT_EQ(Height(246), savedProofDescriptors[0].Height);
		EXPECT_EQ(hashes[1], savedProofDescriptors[0].Hash);
	}

	// endregion
}}
