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

#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/chain/RoundContext.h"
#include "finalization/src/model/FinalizationRoundRange.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "finalization/tests/test/mocks/MockRoundMessageAggregator.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS MultiRoundMessageAggregatorTests

	namespace {
		constexpr auto Default_Min_Round = model::FinalizationRound{ FinalizationEpoch(7), FinalizationPoint(3) };
		constexpr auto Default_Max_Round = model::FinalizationRound{ FinalizationEpoch(7), FinalizationPoint(13) };
		constexpr auto Default_Round_Range = model::FinalizationRoundRange(Default_Min_Round, Default_Max_Round);

		constexpr auto Last_Finalized_Height = Height(123);

		// region TestContext

		struct TestContextOptions {
			uint64_t MaxResponseSize = 10'000'000;
		};

		class TestContext {
		private:
			using RoundMessageAggregatorInitializer = consumer<mocks::MockRoundMessageAggregator&>;

		public:
			TestContext() : TestContext(Default_Min_Round)
			{}

			explicit TestContext(const model::FinalizationRound& round) : TestContext(round, TestContextOptions())
			{}

			TestContext(const model::FinalizationRound& round, const TestContextOptions& options)
					: m_lastFinalizedHash(test::GenerateRandomByteArray<Hash256>()) {
				m_pAggregator = std::make_unique<MultiRoundMessageAggregator>(
						options.MaxResponseSize,
						round,
						model::HeightHashPair{ Last_Finalized_Height, m_lastFinalizedHash },
						[this](const auto& subround) {
							auto pRoundMessageAggregator = std::make_unique<mocks::MockRoundMessageAggregator>(subround);
							if (m_roundMessageAggregatorInitializer)
								m_roundMessageAggregatorInitializer(*pRoundMessageAggregator);

							m_pRoundMessageAggregators.push_back(pRoundMessageAggregator.get());
							return pRoundMessageAggregator;
						});
			}

		public:
			auto& aggregator() {
				return *m_pAggregator;
			}

			auto& roundMessageAggregators() {
				return m_pRoundMessageAggregators;
			}

			auto lastFinalizedHash() const {
				return m_lastFinalizedHash;
			}

		public:
			void setRoundMessageAggregatorInitializer(const RoundMessageAggregatorInitializer& roundMessageAggregatorInitializer) {
				m_roundMessageAggregatorInitializer = roundMessageAggregatorInitializer;
			}

			auto detach() {
				return std::move(m_pAggregator);
			}

		private:
			Hash256 m_lastFinalizedHash;
			RoundMessageAggregatorInitializer m_roundMessageAggregatorInitializer;

			std::vector<mocks::MockRoundMessageAggregator*> m_pRoundMessageAggregators;
			std::unique_ptr<MultiRoundMessageAggregator> m_pAggregator;
		};

		// endregion

		// region test utils

		auto CreateMessage(const model::FinalizationRound& round, Height height) {
			auto pMessage = test::CreateMessage(round);
			pMessage->Data().Height = height;
			return pMessage;
		}

		void AddRoundMessageAggregators(TestContext& context, const std::vector<FinalizationEpoch>& epochDeltas) {
			// Arrange:
			context.aggregator().modifier().setMaxFinalizationRound({
				Default_Min_Round.Epoch + epochDeltas.back(),
				Default_Min_Round.Point
			});

			auto i = 1u;
			for (auto epochDelta : epochDeltas) {
				auto round = model::FinalizationRound{ Default_Min_Round.Epoch + epochDelta, Default_Min_Round.Point };
				context.aggregator().modifier().add(CreateMessage(round, Height(100 + i * 50)));
				++i;
			}

			// Sanity:
			EXPECT_EQ(epochDeltas.size(), context.aggregator().view().size());
		}

		void AddRoundMessageAggregators(
				TestContext& context,
				const std::vector<FinalizationPoint>& pointDeltas,
				size_t numExpectedRoundAggregators = 0) {
			// Arrange:
			context.aggregator().modifier().setMaxFinalizationRound(Default_Min_Round + pointDeltas.back());

			auto i = 1u;
			for (auto pointDelta : pointDeltas) {
				context.aggregator().modifier().add(CreateMessage(Default_Min_Round + pointDelta, Height(100 + i * 50)));
				++i;
			}

			// Sanity:
			numExpectedRoundAggregators = 0 == numExpectedRoundAggregators ? pointDeltas.size() : numExpectedRoundAggregators;
			EXPECT_EQ(numExpectedRoundAggregators, context.aggregator().view().size());
		}

		// endregion
	}

	// region constructor

	namespace {
		void AssertEmptyProperties(
				const MultiRoundMessageAggregatorView& view,
				const model::FinalizationRound& expectedMaxFinalizationRound) {
			EXPECT_EQ(0u, view.size());
			EXPECT_EQ(Default_Min_Round, view.minFinalizationRound());
			EXPECT_EQ(expectedMaxFinalizationRound, view.maxFinalizationRound());
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyAggregator) {
		// Act:
		TestContext context;

		// Assert:
		AssertEmptyProperties(context.aggregator().view(), Default_Min_Round);
	}

	// endregion

	// region setMaxFinalizationRound

	TEST(TEST_CLASS, CanSetMaxFinalizationRoundAboveMin) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregator().modifier().setMaxFinalizationRound(Default_Min_Round + FinalizationPoint(11));

		// Assert:
		AssertEmptyProperties(context.aggregator().view(), Default_Min_Round + FinalizationPoint(11));
	}

	TEST(TEST_CLASS, CanSetMaxFinalizationRoundToMin) {
		// Arrange:
		TestContext context;
		context.aggregator().modifier().setMaxFinalizationRound(Default_Min_Round + FinalizationPoint(11));

		// Act:
		context.aggregator().modifier().setMaxFinalizationRound(Default_Min_Round);

		// Assert:
		AssertEmptyProperties(context.aggregator().view(), Default_Min_Round);
	}

	TEST(TEST_CLASS, CannotSetMaxFinalizationRoundBelowMin) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		EXPECT_THROW(
				context.aggregator().modifier().setMaxFinalizationRound(Default_Min_Round - FinalizationPoint(1)),
				catapult_invalid_argument);
	}

	// endregion

	// region add

	namespace {
		void AssertCannotAddMessage(const model::FinalizationRound& round) {
			// Arrange:
			TestContext context;
			context.aggregator().modifier().setMaxFinalizationRound(Default_Max_Round);

			// Act:
			auto result = context.aggregator().modifier().add(CreateMessage(round, Height(222)));

			// Assert:
			EXPECT_EQ(RoundMessageAggregatorAddResult::Failure_Invalid_Point, result);
			EXPECT_EQ(0u, context.aggregator().view().size());
			EXPECT_EQ(0u, context.roundMessageAggregators().size());
		}

		void AssertCanAddMessage(const model::FinalizationRound& round, RoundMessageAggregatorAddResult expectedAddResult) {
			// Arrange:
			TestContext context;
			context.aggregator().modifier().setMaxFinalizationRound(Default_Max_Round);
			context.setRoundMessageAggregatorInitializer([expectedAddResult](auto& roundMessageAggregator) {
				roundMessageAggregator.setAddResult(expectedAddResult);
			});

			// Act:
			auto result = context.aggregator().modifier().add(CreateMessage(round, Height(222)));

			// Assert:
			EXPECT_EQ(expectedAddResult, result);
			EXPECT_EQ(1u, context.aggregator().view().size());
			ASSERT_EQ(1u, context.roundMessageAggregators().size());

			EXPECT_EQ(round, context.roundMessageAggregators()[0]->round());
			EXPECT_EQ(1u, context.roundMessageAggregators()[0]->numAddCalls());
		}

		void AssertCanAddMessage(const model::FinalizationRound& round) {
			AssertCanAddMessage(round, RoundMessageAggregatorAddResult::Success_Prevote);
		}
	}

	TEST(TEST_CLASS, CannotAddMessageWithPointLessThanMin) {
		AssertCannotAddMessage(Default_Min_Round - FinalizationPoint(1));
	}

	TEST(TEST_CLASS, CannotAddMessageWithPointGreaterThanMax) {
		AssertCannotAddMessage(Default_Max_Round + FinalizationPoint(1));
	}

	TEST(TEST_CLASS, CanAddMessageWithPointAtMin) {
		AssertCanAddMessage(Default_Min_Round);
	}

	TEST(TEST_CLASS, CanAddMessageWithPointBetweenMinAndMax) {
		AssertCanAddMessage(Default_Min_Round + FinalizationPoint(5));
	}

	TEST(TEST_CLASS, CanAddMessageWithPointAtMax) {
		AssertCanAddMessage(Default_Max_Round);
	}

	TEST(TEST_CLASS, CanAddMessageWithPointBetweenMinAndMaxWithNonSuccessResult) {
		// Assert: mock doesn't do any filtering
		AssertCanAddMessage(Default_Min_Round + FinalizationPoint(5), RoundMessageAggregatorAddResult::Failure_Invalid_Height);
	}

	TEST(TEST_CLASS, CanAddMultipleMessagesWithSamePoint) {
		// Arrange:
		TestContext context;
		context.aggregator().modifier().setMaxFinalizationRound(Default_Max_Round);

		// Act:
		for (auto i = 1u; i <= 3; ++i)
			context.aggregator().modifier().add(CreateMessage(Default_Min_Round + FinalizationPoint(5), Height(100 + i * 50)));

		// Assert:
		EXPECT_EQ(1u, context.aggregator().view().size());
		ASSERT_EQ(1u, context.roundMessageAggregators().size());

		EXPECT_EQ(Default_Min_Round + FinalizationPoint(5), context.roundMessageAggregators()[0]->round());
		EXPECT_EQ(3u, context.roundMessageAggregators()[0]->numAddCalls());
	}

	TEST(TEST_CLASS, CanAddMultipleMessagesWithDifferentPoints) {
		// Arrange:
		TestContext context;

		// Act:
		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Assert:
		EXPECT_EQ(3u, context.aggregator().view().size());
		ASSERT_EQ(3u, context.roundMessageAggregators().size());

		auto i = 0u;
		for (auto pointDelta : { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) }) {
			EXPECT_EQ(Default_Min_Round + pointDelta, context.roundMessageAggregators()[i]->round()) << i;
			EXPECT_EQ(1u, context.roundMessageAggregators()[i]->numAddCalls()) << i;
			++i;
		}
	}

	TEST(TEST_CLASS, CanAddMultipleMessagesWithDifferentPointsOutOfOrder) {
		// Arrange:
		TestContext context;

		// Act:
		AddRoundMessageAggregators(context, { FinalizationPoint(5), FinalizationPoint(0), FinalizationPoint(10) });
		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5) }, 3);

		// Assert:
		EXPECT_EQ(3u, context.aggregator().view().size());
		ASSERT_EQ(3u, context.roundMessageAggregators().size());

		auto i = 0u;
		for (auto pointDelta : { FinalizationPoint(5), FinalizationPoint(0), FinalizationPoint(10) }) {
			EXPECT_EQ(Default_Min_Round + pointDelta, context.roundMessageAggregators()[i]->round()) << i;
			EXPECT_EQ(FinalizationPoint(10) == pointDelta ? 1u : 2u, context.roundMessageAggregators()[i]->numAddCalls()) << i;
			++i;
		}
	}

	TEST(TEST_CLASS, CanAddMultipleMessagesWithDifferentEpochs) {
		// Arrange:
		TestContext context;

		// Act:
		AddRoundMessageAggregators(context, { FinalizationEpoch(10), FinalizationEpoch(20), FinalizationEpoch(30) });

		// Assert:
		EXPECT_EQ(3u, context.aggregator().view().size());
		ASSERT_EQ(3u, context.roundMessageAggregators().size());

		auto i = 0u;
		for (auto epochDelta : { FinalizationEpoch(10), FinalizationEpoch(20), FinalizationEpoch(30) }) {
			auto expectedRound = model::FinalizationRound{ Default_Min_Round.Epoch + epochDelta, Default_Min_Round.Point };
			EXPECT_EQ(expectedRound, context.roundMessageAggregators()[i]->round()) << i;
			EXPECT_EQ(1u, context.roundMessageAggregators()[i]->numAddCalls()) << i;
			++i;
		}
	}

	// endregion

	// region tryGetRoundContext

	TEST(TEST_CLASS, TryGetRoundContextReturnsNullptrWhenSpecifiedPointIsUnknown) {
		// Arrange:
		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto aggregatorView = context.aggregator().view();
		const auto* pRoundContext = aggregatorView.tryGetRoundContext(Default_Min_Round + FinalizationPoint(7));

		// Assert:
		EXPECT_FALSE(!!pRoundContext);
	}

	TEST(TEST_CLASS, TryGetRoundContextReturnsValidRoundContextWhenSpecifiedPointIsKnown) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hash](auto& roundMessageAggregator) {
			roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, roundMessageAggregator.round().Point.unwrap());
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto aggregatorView = context.aggregator().view();
		const auto* pRoundContext = aggregatorView.tryGetRoundContext(Default_Min_Round + FinalizationPoint(5));

		// Assert:
		ASSERT_TRUE(!!pRoundContext);
		EXPECT_EQ(3u + 5, pRoundContext->weights({ Height(222), hash }).Prevote);
	}

	TEST(TEST_CLASS, TryGetRoundContextReturnsValidRoundContextWhenSpecifiedPointIsKnownAcrossMultipleEpochs) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hash](auto& roundMessageAggregator) {
			roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, roundMessageAggregator.round().Epoch.unwrap());
		});

		AddRoundMessageAggregators(context, { FinalizationEpoch(10), FinalizationEpoch(20), FinalizationEpoch(30) });

		// Act:
		auto aggregatorView = context.aggregator().view();
		const auto* pRoundContext = aggregatorView.tryGetRoundContext({
			Default_Min_Round.Epoch + FinalizationEpoch(20),
			Default_Min_Round.Point
		});

		// Assert:
		ASSERT_TRUE(!!pRoundContext);
		EXPECT_EQ(7u + 20, pRoundContext->weights({ Height(222), hash }).Prevote);
	}

	// endregion

	// region findEstimate

	namespace {
		model::HeightHashPair FindPreviousEstimate(const MultiRoundMessageAggregator& context) {
			return context.view().findEstimate(Default_Max_Round - FinalizationPoint(1));
		}

		model::HeightHashPair FindCurrentEstimate(const MultiRoundMessageAggregator& context) {
			return context.view().findEstimate(Default_Max_Round);
		}
	}

	TEST(TEST_CLASS, FindEstimateReturnsInitializedValueWhenEmpty) {
		// Arrange:
		TestContext context;

		// Act:
		auto previousEstimate = FindPreviousEstimate(context.aggregator());
		auto currentEstimate = FindCurrentEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(model::HeightHashPair({ Last_Finalized_Height, context.lastFinalizedHash() }), previousEstimate);
		EXPECT_EQ(model::HeightHashPair({ Last_Finalized_Height, context.lastFinalizedHash() }), currentEstimate);
	}

	TEST(TEST_CLASS, FindEstimateReturnsInitializedValueWhenNoPreviousRoundHasAnEstimate) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hash](auto& roundMessageAggregator) {
			// - set estimate for last aggregator only
			if (Default_Max_Round == roundMessageAggregator.round())
				roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto previousEstimate = FindPreviousEstimate(context.aggregator());
		auto currentEstimate = FindCurrentEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(model::HeightHashPair({ Last_Finalized_Height, context.lastFinalizedHash() }), previousEstimate);
		EXPECT_EQ(model::HeightHashPair({ Height(222), hash }), currentEstimate);
	}

	TEST(TEST_CLASS, FindEstimateReturnsPreviousRoundValueWhenMostRecentPreviousRoundHasAnEstimate) {
		// Arrange:
		std::vector<Hash256> hashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			// - set estimate for all aggregators
			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashes.push_back(hash);

			auto height = Height(roundMessageAggregator.round().Point.unwrap() * 100);
			roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto previousEstimate = FindPreviousEstimate(context.aggregator());
		auto currentEstimate = FindCurrentEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(model::HeightHashPair({ Height(800), hashes[1] }), previousEstimate);
		EXPECT_EQ(model::HeightHashPair({ Height(1300), hashes[2] }), currentEstimate);
	}

	TEST(TEST_CLASS, FindEstimateReturnsPreviousRoundValueWhenAnyPreviousRoundHasAnEstimate) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hash](auto& roundMessageAggregator) {
			// - set estimate for first aggregator only
			if (Default_Min_Round == roundMessageAggregator.round())
				roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto previousEstimate = FindPreviousEstimate(context.aggregator());
		auto currentEstimate = FindCurrentEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(model::HeightHashPair({ Height(222), hash }), previousEstimate);
		EXPECT_EQ(model::HeightHashPair({ Height(222), hash }), currentEstimate);
	}

	// endregion

	// region tryFindBestPrecommit

	namespace {
		void AssertUnset(const BestPrecommitDescriptor& descriptor) {
			EXPECT_EQ(model::FinalizationRound(), descriptor.Round);
			EXPECT_EQ(model::HeightHashPair(), descriptor.Target);
			EXPECT_TRUE(descriptor.Proof.empty());
		}

		void AssertSet(
				const BestPrecommitDescriptor& descriptor,
				FinalizationPoint expectedPointDelta,
				const model::HeightHashPair& expectedTarget,
				size_t expectedNumProofMessages) {
			EXPECT_EQ(Default_Min_Round + expectedPointDelta, descriptor.Round);
			EXPECT_EQ(expectedTarget, descriptor.Target);
			EXPECT_EQ(expectedNumProofMessages, descriptor.Proof.size());

			for (const auto& pMessage : descriptor.Proof)
				EXPECT_EQ(Default_Min_Round + expectedPointDelta, pMessage->Data().StepIdentifier.Round());
		}
	}

	TEST(TEST_CLASS, BestPrecommitDoesNotExistWhenEmpty) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		AssertUnset(context.aggregator().view().tryFindBestPrecommit());
	}

	TEST(TEST_CLASS, BestPrecommitDoesNotExistWhenNoRoundHasBestPrecommit) {
		// Arrange:
		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act + Assert:
		AssertUnset(context.aggregator().view().tryFindBestPrecommit());
	}

	TEST(TEST_CLASS, BestPrecommitExistsWhenCurrentRoundHasBestPrecommit) {
		// Arrange:
		std::vector<Hash256> hashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashes.push_back(hash);

			auto height = Height(roundMessageAggregator.round().Point.unwrap() * 100);
			roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);
			roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);

			RoundMessageAggregator::UnknownMessages messages;
			for (auto i = 0u; i < 3; ++i)
				messages.push_back(test::CreateMessage(roundMessageAggregator.round()));

			roundMessageAggregator.setMessages(std::move(messages));
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act + Assert:
		AssertSet(context.aggregator().view().tryFindBestPrecommit(), FinalizationPoint(10), { Height(1300), hashes[2] }, 3);
	}

	TEST(TEST_CLASS, BestPrecommitExistsWhenAnyPreviousRoundHasBestPrecommit) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hash](auto& roundMessageAggregator) {
			// - set precommit for first aggregator only
			auto numMessages = 3u;
			if (Default_Min_Round == roundMessageAggregator.round()) {
				roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, 750);
				roundMessageAggregator.roundContext().acceptPrecommit(Height(222), hash, 750);

				numMessages = 4;
			}

			RoundMessageAggregator::UnknownMessages messages;
			for (auto i = 0u; i < numMessages; ++i)
				messages.push_back(test::CreateMessage(roundMessageAggregator.round()));

			roundMessageAggregator.setMessages(std::move(messages));
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act + Assert:
		AssertSet(context.aggregator().view().tryFindBestPrecommit(), FinalizationPoint(0), { Height(222), hash }, 4);
	}

	// endregion

	// region shortHashes

	TEST(TEST_CLASS, ShortHashesReturnsNoShortHashesWhenAggregtorIsEmpty) {
		// Arrange:
		TestContext context;

		// Act:
		auto shortHashes = context.aggregator().view().shortHashes();

		// Assert:
		EXPECT_TRUE(shortHashes.empty());
	}

	TEST(TEST_CLASS, ShortHashesReturnsShortHashesForAllMessages) {
		// Arrange:
		utils::ShortHashesSet seededShortHashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&seededShortHashes](auto& roundMessageAggregator) {
			auto numHashes = roundMessageAggregator.round().Point.unwrap();
			auto shortHashes = test::GenerateRandomDataVector<utils::ShortHash>(numHashes);
			seededShortHashes.insert(shortHashes.cbegin(), shortHashes.cend());

			auto shortHashesRange = model::ShortHashRange::CopyFixed(reinterpret_cast<const uint8_t*>(shortHashes.data()), numHashes);
			roundMessageAggregator.setShortHashes(std::move(shortHashesRange));
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto shortHashes = context.aggregator().view().shortHashes();

		// Assert:
		EXPECT_EQ(15u + Default_Min_Round.Point.unwrap() * 3, shortHashes.size());
		EXPECT_EQ(seededShortHashes, utils::ShortHashesSet(shortHashes.cbegin(), shortHashes.cend()));

		// - returned range is contiguous (due to limitation in PacketPayloadBuilder)
		EXPECT_NO_THROW(shortHashes.data());
	}

	// endregion

	// region unknownMessages

	namespace {
		auto ToShortHashes(const std::vector<std::shared_ptr<const model::FinalizationMessage>>& messages) {
			utils::ShortHashesSet shortHashes;
			for (const auto& pMessage : messages)
				shortHashes.insert(utils::ToShortHash(model::CalculateMessageHash(*pMessage)));

			return shortHashes;
		}

		auto SeedUnknownMessages(TestContext& context) {
			std::vector<utils::ShortHash> shortHashes;
			context.setRoundMessageAggregatorInitializer([&shortHashes](auto& roundMessageAggregator) {
				RoundMessageAggregator::UnknownMessages messages;
				for (auto i = 0u; i < 3; ++i) {
					messages.push_back(test::CreateMessage(roundMessageAggregator.round()));
					shortHashes.push_back(utils::ToShortHash(model::CalculateMessageHash(*messages.back())));
				}

				roundMessageAggregator.setMessages(std::move(messages));
			});

			return shortHashes;
		}

		template<typename TAction>
		void RunSeededAggregatorTest(TAction action) {
			// Arrange:
			TestContext context;

			auto shortHashes = SeedUnknownMessages(context);
			AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

			// Sanity:
			EXPECT_EQ(9u, shortHashes.size());

			// Act + Assert:
			action(context.aggregator().view(), shortHashes);
		}
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsNoMessagesWhenAggregatorIsEmpty) {
		// Arrange:
		TestContext context;

		// Act:
		auto unknownMessages = context.aggregator().view().unknownMessages(Default_Round_Range, {});

		// Assert:
		EXPECT_TRUE(unknownMessages.empty());
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsAllMessagesWhenFilterIsEmpty) {
		// Arrange:
		RunSeededAggregatorTest([](const auto& aggregator, const auto& seededShortHashes) {
			// Act:
			auto unknownMessages = aggregator.unknownMessages(Default_Round_Range, {});

			// Assert:
			EXPECT_EQ(9u, unknownMessages.size());
			EXPECT_EQ(utils::ShortHashesSet(seededShortHashes.cbegin(), seededShortHashes.cend()), ToShortHashes(unknownMessages));
		});
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsMessagesWithinSpecifiedRange) {
		// Arrange:
		TestContext context;

		auto seededShortHashes = SeedUnknownMessages(context);
		AddRoundMessageAggregators(context, {
			FinalizationPoint(0), FinalizationPoint(3), FinalizationPoint(5), FinalizationPoint(7), FinalizationPoint(10)
		});

		// Sanity:
		EXPECT_EQ(15u, seededShortHashes.size());

		// Act:
		auto unknownMessages = context.aggregator().view().unknownMessages(
				{ Default_Min_Round + FinalizationPoint(3), Default_Min_Round + FinalizationPoint(7) },
				{});

		// Assert: should return messages from points { +3, +5, +7 } only
		EXPECT_EQ(9u, unknownMessages.size());
		EXPECT_EQ(utils::ShortHashesSet(seededShortHashes.cbegin() + 3, seededShortHashes.cbegin() + 12), ToShortHashes(unknownMessages));
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsAllMessagesNotInFilter) {
		// Arrange:
		RunSeededAggregatorTest([](const auto& aggregator, const auto& seededShortHashes) {
			// Act:
			auto unknownMessages = aggregator.unknownMessages(Default_Round_Range, {
				seededShortHashes[0], seededShortHashes[1], seededShortHashes[4], seededShortHashes[6], seededShortHashes[7]
			});

			// Assert:
			EXPECT_EQ(4u, unknownMessages.size());
			EXPECT_EQ(
					utils::ShortHashesSet({ seededShortHashes[2], seededShortHashes[3], seededShortHashes[5], seededShortHashes[8] }),
					ToShortHashes(unknownMessages));
		});
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsNoMessagesWhenAllMessagesAreKnown) {
		// Arrange:
		RunSeededAggregatorTest([](const auto& aggregator, const auto& seededShortHashes) {
			// Act:
			auto unknownMessages = aggregator.unknownMessages(
					Default_Round_Range,
					utils::ShortHashesSet(seededShortHashes.cbegin(), seededShortHashes.cend()));

			// Assert:
			EXPECT_TRUE(unknownMessages.empty());
		});
	}

	namespace {
		template<typename TAction>
		void RunMaxResponseSizeTests(TAction action) {
			// Arrange: determine message size from a generated message
			auto messageSize = test::CreateMessage(Default_Min_Round)->Size;

			// Assert:
			action(2, 3 * messageSize - 1);
			action(3, 3 * messageSize);
			action(3, 3 * messageSize + 1);

			action(3, 4 * messageSize - 1);
			action(4, 4 * messageSize);
		}
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsMessagesWithTotalSizeOfAtMostMaxResponseSize) {
		// Arrange:
		RunMaxResponseSizeTests([](uint32_t numExpectedMessages, size_t maxResponseSize) {
			TestContextOptions options;
			options.MaxResponseSize = maxResponseSize;
			TestContext context(Default_Min_Round, options);

			auto seededShortHashes = SeedUnknownMessages(context);
			AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

			// Act:
			auto unknownMessages = context.aggregator().view().unknownMessages(Default_Round_Range, {});

			// Assert:
			EXPECT_EQ(numExpectedMessages, unknownMessages.size());
			EXPECT_EQ(
					utils::ShortHashesSet(seededShortHashes.cbegin(), seededShortHashes.cbegin() + numExpectedMessages),
					ToShortHashes(unknownMessages));
		});
	}

	// endregion

	// region prune

	namespace {
		void AssertMinMaxFinalizationRounds(
				const MultiRoundMessageAggregatorView& view,
				size_t expectedSize,
				const model::FinalizationRound& expectedMinRound,
				const model::FinalizationRound& expectedMaxRound) {
			EXPECT_EQ(expectedSize, view.size());
			EXPECT_EQ(expectedMinRound, view.minFinalizationRound());
			EXPECT_EQ(expectedMaxRound, view.maxFinalizationRound());
		}
	}

	TEST(TEST_CLASS, PruneUpdatesMinFinalizationRoundWhenEmpty) {
		// Arrange:
		TestContext context;
		context.aggregator().modifier().setMaxFinalizationRound(Default_Max_Round);

		// Sanity:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 0, Default_Min_Round, Default_Max_Round);

		// Act:
		context.aggregator().modifier().prune(FinalizationEpoch(100));

		// Assert:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 0, Default_Max_Round, Default_Max_Round);
	}

	TEST(TEST_CLASS, PruneRemovesAllRoundsWhenAllAreBelowPruneEpoch) {
		// Arrange:
		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationPoint(1), FinalizationPoint(5), FinalizationPoint(10) });

		// Sanity:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 3, Default_Min_Round, Default_Max_Round);

		// Act:
		context.aggregator().modifier().prune(FinalizationEpoch(100));

		// Assert:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 0, Default_Max_Round, Default_Max_Round);
	}

	TEST(TEST_CLASS, PruneRemovesNoRoundsWhenAllAreAbovePruneEpoch) {
		// Arrange:
		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationPoint(1), FinalizationPoint(5), FinalizationPoint(10) });

		// Sanity:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 3, Default_Min_Round, Default_Max_Round);

		// Act:
		context.aggregator().modifier().prune(Default_Min_Round.Epoch - FinalizationEpoch(1));

		// Assert:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 3, Default_Min_Round + FinalizationPoint(1), Default_Max_Round);
	}

	TEST(TEST_CLASS, PruneOnlyRemovesRoundsWithEpochLessThanPruneEpoch_MatchingRounds) {
		// Arrange:
		constexpr auto Epoch = Default_Min_Round.Epoch;
		constexpr auto Point_Min = FinalizationPoint();
		constexpr auto Point_Max = FinalizationPoint(std::numeric_limits<uint32_t>::max());
		constexpr auto Max_Round = model::FinalizationRound{ Epoch + FinalizationEpoch(10), Default_Min_Round.Point };

		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationEpoch(1), FinalizationEpoch(5), FinalizationEpoch(10) });
		context.aggregator().modifier().add(CreateMessage({ Epoch + FinalizationEpoch(4), Point_Max }, Height(100)));
		context.aggregator().modifier().add(CreateMessage({ Epoch + FinalizationEpoch(5), Point_Min }, Height(100)));

		// Sanity:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 5, Default_Min_Round, Max_Round);

		// Act:
		context.aggregator().modifier().prune(Epoch + FinalizationEpoch(5));

		// Assert:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 3, { Epoch + FinalizationEpoch(5), Point_Min }, Max_Round);
	}

	TEST(TEST_CLASS, PruneOnlyRemovesRoundsWithEpochLessThanPruneEpoch_NoMatchingRounds) {
		// Arrange:
		constexpr auto Epoch = Default_Min_Round.Epoch;
		constexpr auto Max_Round = model::FinalizationRound{ Epoch + FinalizationEpoch(10), Default_Min_Round.Point };

		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationEpoch(1), FinalizationEpoch(5), FinalizationEpoch(10) });

		// Sanity:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 3, Default_Min_Round, Max_Round);

		// Act:
		context.aggregator().modifier().prune(Epoch + FinalizationEpoch(9));

		// Assert:
		AssertMinMaxFinalizationRounds(context.aggregator().view(), 1, Max_Round, Max_Round);
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return TestContext().detach();
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}
