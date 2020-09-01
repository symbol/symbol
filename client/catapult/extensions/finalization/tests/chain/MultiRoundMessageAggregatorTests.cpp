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

#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/chain/RoundContext.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "finalization/tests/test/mocks/MockRoundMessageAggregator.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS MultiRoundMessageAggregatorTests

	namespace {
		constexpr auto Default_Min_FP = FinalizationPoint(3);
		constexpr auto Default_Max_FP = FinalizationPoint(13);

		constexpr auto Last_Finalized_Height = Height(123);

		// region TestContext

		struct TestContextOptions {
			uint64_t MaxResponseSize = 10'000'000;
		};

		class TestContext {
		private:
			using RoundMessageAggregatorInitializer = consumer<mocks::MockRoundMessageAggregator&>;

		public:
			TestContext() : TestContext(Default_Min_FP)
			{}

			explicit TestContext(FinalizationPoint point) : TestContext(point, TestContextOptions())
			{}

			TestContext(FinalizationPoint point, const TestContextOptions& options)
					: m_lastFinalizedHash(test::GenerateRandomByteArray<Hash256>()) {
				m_pAggregator = std::make_unique<MultiRoundMessageAggregator>(
						options.MaxResponseSize,
						point,
						model::HeightHashPair{ Last_Finalized_Height, m_lastFinalizedHash },
						[this](auto roundPoint, auto height) {
							auto pRoundMessageAggregator = std::make_unique<mocks::MockRoundMessageAggregator>(roundPoint, height);
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

		void AddRoundMessageAggregators(TestContext& context, const std::vector<FinalizationPoint>& pointDeltas) {
			// Arrange:
			context.aggregator().modifier().setMaxFinalizationPoint(Default_Max_FP);

			auto i = 1u;
			for (auto pointDelta : pointDeltas) {
				context.aggregator().modifier().add(test::CreateMessage(Default_Min_FP + pointDelta, Height(100 + i * 50)));
				++i;
			}

			// Sanity:
			EXPECT_EQ(3u, context.aggregator().view().size());
		}

		// endregion
	}

	// region constructor

	namespace {
		void AssertEmptyProperties(const MultiRoundMessageAggregatorView& view, FinalizationPoint expectedMaxFinalizationPoint) {
			EXPECT_EQ(0u, view.size());
			EXPECT_EQ(Default_Min_FP, view.minFinalizationPoint());
			EXPECT_EQ(expectedMaxFinalizationPoint, view.maxFinalizationPoint());
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyAggregator) {
		// Act:
		TestContext context;

		// Assert:
		AssertEmptyProperties(context.aggregator().view(), Default_Min_FP);
	}

	// endregion

	// region setMaxFinalizationPoint

	TEST(TEST_CLASS, CanSetMaxFinalizationPointAboveMin) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregator().modifier().setMaxFinalizationPoint(Default_Min_FP + FinalizationPoint(11));

		// Assert:
		AssertEmptyProperties(context.aggregator().view(), Default_Min_FP + FinalizationPoint(11));
	}

	TEST(TEST_CLASS, CanSetMaxFinalizationPointToMin) {
		// Arrange:
		TestContext context;
		context.aggregator().modifier().setMaxFinalizationPoint(Default_Min_FP + FinalizationPoint(11));

		// Act:
		context.aggregator().modifier().setMaxFinalizationPoint(Default_Min_FP);

		// Assert:
		AssertEmptyProperties(context.aggregator().view(), Default_Min_FP);
	}

	TEST(TEST_CLASS, CannotSetMaxFinalizationPointBelowMin) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		EXPECT_THROW(
				context.aggregator().modifier().setMaxFinalizationPoint(Default_Min_FP - FinalizationPoint(1)),
				catapult_invalid_argument);
	}

	// endregion

	// region add

	namespace {
		void AssertCannotAddMessage(FinalizationPoint point) {
			// Arrange:
			TestContext context;
			context.aggregator().modifier().setMaxFinalizationPoint(Default_Max_FP);

			// Act:
			auto result = context.aggregator().modifier().add(test::CreateMessage(point, Height(222)));

			// Assert:
			EXPECT_EQ(RoundMessageAggregatorAddResult::Failure_Invalid_Point, result);
			EXPECT_EQ(0u, context.aggregator().view().size());
			EXPECT_EQ(0u, context.roundMessageAggregators().size());
		}

		void AssertCanAddMessage(FinalizationPoint point, RoundMessageAggregatorAddResult expectedAddResult) {
			// Arrange:
			TestContext context;
			context.aggregator().modifier().setMaxFinalizationPoint(Default_Max_FP);
			context.setRoundMessageAggregatorInitializer([expectedAddResult](auto& roundMessageAggregator) {
				roundMessageAggregator.setAddResult(expectedAddResult);
			});

			// Act:
			auto result = context.aggregator().modifier().add(test::CreateMessage(point, Height(222)));

			// Assert:
			EXPECT_EQ(expectedAddResult, result);
			EXPECT_EQ(1u, context.aggregator().view().size());
			ASSERT_EQ(1u, context.roundMessageAggregators().size());

			EXPECT_EQ(point, context.roundMessageAggregators()[0]->point());
			EXPECT_EQ(Height(222), context.roundMessageAggregators()[0]->height());
			EXPECT_EQ(1u, context.roundMessageAggregators()[0]->numAddCalls());
		}

		void AssertCanAddMessage(FinalizationPoint point) {
			AssertCanAddMessage(point, RoundMessageAggregatorAddResult::Success_Prevote);
		}
	}

	TEST(TEST_CLASS, CannotAddMessageWithPointLessThanMin) {
		AssertCannotAddMessage(Default_Min_FP - FinalizationPoint(1));
	}

	TEST(TEST_CLASS, CannotAddMessageWithPointGreaterThanMax) {
		AssertCannotAddMessage(Default_Max_FP + FinalizationPoint(1));
	}

	TEST(TEST_CLASS, CanAddMessageWithPointAtMin) {
		AssertCanAddMessage(Default_Min_FP);
	}

	TEST(TEST_CLASS, CanAddMessageWithPointBetweenMinAndMax) {
		AssertCanAddMessage(Default_Min_FP + FinalizationPoint(5));
	}

	TEST(TEST_CLASS, CanAddMessageWithPointAtMax) {
		AssertCanAddMessage(Default_Max_FP);
	}

	TEST(TEST_CLASS, CanAddMessageWithPointBetweenMinAndMaxWithNonSuccessResult) {
		// Assert: mock doesn't do any filtering
		AssertCanAddMessage(Default_Min_FP + FinalizationPoint(5), RoundMessageAggregatorAddResult::Failure_Invalid_Height);
	}

	TEST(TEST_CLASS, CanAddMultipleMessagesWithSamePoint) {
		// Arrange:
		TestContext context;
		context.aggregator().modifier().setMaxFinalizationPoint(Default_Max_FP);

		// Act:
		for (auto i = 1u; i <= 3; ++i)
			context.aggregator().modifier().add(test::CreateMessage(Default_Min_FP + FinalizationPoint(5), Height(100 + i * 50)));

		// Assert:
		EXPECT_EQ(1u, context.aggregator().view().size());
		ASSERT_EQ(1u, context.roundMessageAggregators().size());

		EXPECT_EQ(Default_Min_FP + FinalizationPoint(5), context.roundMessageAggregators()[0]->point());
		EXPECT_EQ(Height(150), context.roundMessageAggregators()[0]->height());
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
			EXPECT_EQ(Default_Min_FP + pointDelta, context.roundMessageAggregators()[i]->point()) << i;
			EXPECT_EQ(Height(100 + (i + 1) * 50), context.roundMessageAggregators()[i]->height()) << i;
			EXPECT_EQ(1u, context.roundMessageAggregators()[i]->numAddCalls()) << i;
			++i;
		}
	}

	TEST(TEST_CLASS, CanAddMultipleMessagesWithDifferentPointsOutOfOrder) {
		// Arrange:
		TestContext context;

		// Act:
		AddRoundMessageAggregators(context, { FinalizationPoint(5), FinalizationPoint(0), FinalizationPoint(10) });
		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5) });

		// Assert:
		EXPECT_EQ(3u, context.aggregator().view().size());
		ASSERT_EQ(3u, context.roundMessageAggregators().size());

		auto i = 0u;
		for (auto pointDelta : { FinalizationPoint(5), FinalizationPoint(0), FinalizationPoint(10) }) {
			EXPECT_EQ(Default_Min_FP + pointDelta, context.roundMessageAggregators()[i]->point()) << i;
			EXPECT_EQ(Height(100 + (i + 1) * 50), context.roundMessageAggregators()[i]->height()) << i;
			EXPECT_EQ(FinalizationPoint(10) == pointDelta ? 1u : 2u, context.roundMessageAggregators()[i]->numAddCalls()) << i;
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
		const auto* pRoundContext = aggregatorView.tryGetRoundContext(Default_Min_FP + FinalizationPoint(7));

		// Assert:
		EXPECT_FALSE(!!pRoundContext);
	}

	TEST(TEST_CLASS, TryGetRoundContextReturnsValidRoundContextWhenSpecifiedPointIsKnown) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hash](auto& roundMessageAggregator) {
			roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, roundMessageAggregator.point().unwrap());
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto aggregatorView = context.aggregator().view();
		const auto* pRoundContext = aggregatorView.tryGetRoundContext(Default_Min_FP + FinalizationPoint(5));

		// Assert:
		ASSERT_TRUE(!!pRoundContext);
		EXPECT_EQ(8u, pRoundContext->weights({ Height(222), hash }).Prevote);
	}

	// endregion

	// region findEstimate

	namespace {
		model::HeightHashPair FindPreviousEstimate(const MultiRoundMessageAggregator& context) {
			return context.view().findEstimate(Default_Max_FP - FinalizationPoint(1));
		}

		model::HeightHashPair FindCurrentEstimate(const MultiRoundMessageAggregator& context) {
			return context.view().findEstimate(Default_Max_FP);
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
			if (Default_Max_FP == roundMessageAggregator.point())
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

			roundMessageAggregator.roundContext().acceptPrevote(Height(roundMessageAggregator.point().unwrap() * 100), &hash, 1, 750);
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
			if (Default_Min_FP == roundMessageAggregator.point())
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
			EXPECT_EQ(FinalizationPoint(0), descriptor.Point);
			EXPECT_EQ(model::HeightHashPair(), descriptor.Target);
			EXPECT_TRUE(descriptor.Proof.empty());
		}

		void AssertSet(
				const BestPrecommitDescriptor& descriptor,
				FinalizationPoint expectedPointDelta,
				const model::HeightHashPair& expectedTarget,
				size_t expectedNumProofMessages) {
			EXPECT_EQ(Default_Min_FP + expectedPointDelta, descriptor.Point);
			EXPECT_EQ(expectedTarget, descriptor.Target);
			EXPECT_EQ(expectedNumProofMessages, descriptor.Proof.size());

			for (const auto& pMessage : descriptor.Proof)
				EXPECT_EQ(Default_Min_FP + expectedPointDelta, pMessage->StepIdentifier.Point);
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

			auto height = Height(roundMessageAggregator.point().unwrap() * 100);
			roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);
			roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);

			RoundMessageAggregator::UnknownMessages messages;
			for (auto i = 0u; i < 3; ++i)
				messages.push_back(test::CreateMessage(roundMessageAggregator.point()));

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
			if (Default_Min_FP == roundMessageAggregator.point()) {
				roundMessageAggregator.roundContext().acceptPrevote(Height(222), &hash, 1, 750);
				roundMessageAggregator.roundContext().acceptPrecommit(Height(222), hash, 750);

				numMessages = 4;
			}

			RoundMessageAggregator::UnknownMessages messages;
			for (auto i = 0u; i < numMessages; ++i)
				messages.push_back(test::CreateMessage(roundMessageAggregator.point()));

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
			auto numHashes = roundMessageAggregator.point().unwrap();
			auto shortHashes = test::GenerateRandomDataVector<utils::ShortHash>(numHashes);
			seededShortHashes.insert(shortHashes.cbegin(), shortHashes.cend());

			auto shortHashesRange = model::ShortHashRange::CopyFixed(reinterpret_cast<const uint8_t*>(shortHashes.data()), numHashes);
			roundMessageAggregator.setShortHashes(std::move(shortHashesRange));
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		auto shortHashes = context.aggregator().view().shortHashes();

		// Assert:
		EXPECT_EQ(15u + Default_Min_FP.unwrap() * 3, shortHashes.size());
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
					messages.push_back(test::CreateMessage(roundMessageAggregator.point()));
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
		auto unknownMessages = context.aggregator().view().unknownMessages(Default_Min_FP, {});

		// Assert:
		EXPECT_TRUE(unknownMessages.empty());
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsAllMessagesWhenFilterIsEmpty) {
		// Arrange:
		RunSeededAggregatorTest([](const auto& aggregator, const auto& seededShortHashes) {
			// Act:
			auto unknownMessages = aggregator.unknownMessages(Default_Min_FP, {});

			// Assert:
			EXPECT_EQ(9u, unknownMessages.size());
			EXPECT_EQ(utils::ShortHashesSet(seededShortHashes.cbegin(), seededShortHashes.cend()), ToShortHashes(unknownMessages));
		});
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsMessagesWithPointNoGreaterThanSpecifiedPoint) {
		// Arrange:
		RunSeededAggregatorTest([](const auto& aggregator, const auto& seededShortHashes) {
			// Act:
			auto unknownMessages = aggregator.unknownMessages(Default_Min_FP + FinalizationPoint(5), {});

			// Assert: should return messages from points { +5, +10 } only
			EXPECT_EQ(6u, unknownMessages.size());
			EXPECT_EQ(utils::ShortHashesSet(seededShortHashes.cbegin() + 3, seededShortHashes.cend()), ToShortHashes(unknownMessages));
		});
	}

	TEST(TEST_CLASS, UnknownMessagesReturnsAllMessagesNotInFilter) {
		// Arrange:
		RunSeededAggregatorTest([](const auto& aggregator, const auto& seededShortHashes) {
			// Act:
			auto unknownMessages = aggregator.unknownMessages(Default_Min_FP, {
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
					Default_Min_FP,
					utils::ShortHashesSet(seededShortHashes.cbegin(), seededShortHashes.cend()));

			// Assert:
			EXPECT_TRUE(unknownMessages.empty());
		});
	}

	namespace {
		template<typename TAction>
		void RunMaxResponseSizeTests(TAction action) {
			// Arrange: determine message size from a generated message
			auto messageSize = test::CreateMessage(Default_Min_FP)->Size;

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
			TestContext context(Default_Min_FP, options);

			auto seededShortHashes = SeedUnknownMessages(context);
			AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

			// Act:
			auto unknownMessages = context.aggregator().view().unknownMessages(Default_Min_FP, {});

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
		void AssertMinMaxFinalizationPoints(
				const MultiRoundMessageAggregatorView& view,
				FinalizationPoint expectedMinPoint,
				FinalizationPoint expectedMaxPoint) {
			EXPECT_EQ(expectedMinPoint, view.minFinalizationPoint());
			EXPECT_EQ(expectedMaxPoint, view.maxFinalizationPoint());
		}
	}

	TEST(TEST_CLASS, PruneHasNoEffectWhenEmpty) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(0u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Last_Finalized_Height, context.lastFinalizedHash() }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Min_FP, Default_Min_FP);
	}

	TEST(TEST_CLASS, PruneHasNoEffectWhenNoRoundHasBestPrecommit) {
		// Arrange:
		TestContext context;
		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(3u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Last_Finalized_Height, context.lastFinalizedHash() }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Min_FP, Default_Max_FP);
	}

	TEST(TEST_CLASS, PruneHasEffectWhenAllRoundsHaveBestPrecommit) {
		// Arrange:
		std::vector<Hash256> hashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashes.push_back(hash);

			auto height = Height(roundMessageAggregator.point().unwrap() * 100);
			roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);
			roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(1u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Height(800), hashes[1] }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Max_FP, Default_Max_FP);
	}

	TEST(TEST_CLASS, PruneHasEffectWhenCurrentRoundHasBestPrecommitAndAllPreviousRoundsHaveEstimate) {
		// Arrange:
		std::vector<Hash256> hashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashes.push_back(hash);

			// - set estimate for all aggregators
			auto height = Height(roundMessageAggregator.point().unwrap() * 100);
			roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);

			// - set precommit for last aggregator only
			if (Default_Max_FP == roundMessageAggregator.point())
				roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(1u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Height(800), hashes[1] }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Max_FP, Default_Max_FP);
	}

	TEST(TEST_CLASS, PruneHasEffectWhenCurrentRoundHasBestPrecommitAndSinglePreviousRoundHasEstimate) {
		// Arrange:
		std::vector<Hash256> hashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashes.push_back(hash);

			// - set estimate for first and last aggregators
			auto height = Height(roundMessageAggregator.point().unwrap() * 100);
			if (Default_Min_FP == roundMessageAggregator.point() || Default_Max_FP == roundMessageAggregator.point())
				roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);

			// - set precommit for last aggregator only
			if (Default_Max_FP == roundMessageAggregator.point())
				roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(1u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Height(300), hashes[0] }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Max_FP, Default_Max_FP);
	}

	TEST(TEST_CLASS, PruneHasEffectWhenCurrentRoundHasBestPrecommitAndNoPreviousRoundHasEstimate) {
		// Arrange:
		TestContext context;
		context.setRoundMessageAggregatorInitializer([](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();

			// - set precommit for last aggregator only
			if (Default_Max_FP == roundMessageAggregator.point()) {
				auto height = Height(roundMessageAggregator.point().unwrap() * 100);
				roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);
				roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);
			}
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(1u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Last_Finalized_Height, context.lastFinalizedHash() }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Max_FP, Default_Max_FP);
	}

	TEST(TEST_CLASS, PruneHasEffectWhenPreviousRoundHasBestPrecommit) {
		// Arrange:
		std::vector<Hash256> hashes;
		TestContext context;
		context.setRoundMessageAggregatorInitializer([&hashes](auto& roundMessageAggregator) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashes.push_back(hash);

			// - set estimate for all aggregators
			auto height = Height(roundMessageAggregator.point().unwrap() * 100);
			roundMessageAggregator.roundContext().acceptPrevote(height, &hash, 1, 750);

			// - set precommit for second aggregator only
			if (Default_Min_FP + FinalizationPoint(5) == roundMessageAggregator.point())
				roundMessageAggregator.roundContext().acceptPrecommit(height, hash, 750);
		});

		AddRoundMessageAggregators(context, { FinalizationPoint(0), FinalizationPoint(5), FinalizationPoint(10) });

		// Act:
		context.aggregator().modifier().prune();
		auto estimate = FindPreviousEstimate(context.aggregator());

		// Assert:
		EXPECT_EQ(2u, context.aggregator().view().size());
		EXPECT_EQ(model::HeightHashPair({ Height(800), hashes[1] }), estimate);

		AssertMinMaxFinalizationPoints(context.aggregator().view(), Default_Min_FP + FinalizationPoint(5), Default_Max_FP);
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
