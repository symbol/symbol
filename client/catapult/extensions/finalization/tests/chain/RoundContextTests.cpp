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

#include "finalization/src/chain/RoundContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS RoundContextTests

	// region RoundContext::Weights operators (for use in test macros)

	constexpr bool operator==(const RoundContext::Weights& lhs, const RoundContext::Weights& rhs);
	constexpr bool operator==(const RoundContext::Weights& lhs, const RoundContext::Weights& rhs) {
		return lhs.Prevote == rhs.Prevote && lhs.Precommit == rhs.Precommit;
	}

	std::ostream& operator<<(std::ostream& out, const RoundContext::Weights& weights);
	std::ostream& operator<<(std::ostream& out, const RoundContext::Weights& weights) {
		out << "(prevote " << weights.Prevote << ", precommit " << weights.Precommit << ")";
		return out;
	}

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateContext) {
		// Act:
		RoundContext context(1000, 670);

		// Assert:
		EXPECT_EQ(0u, context.size());
		EXPECT_FALSE(context.tryFindBestPrevote().second);
		EXPECT_FALSE(context.tryFindBestPrecommit().second);
		EXPECT_FALSE(context.tryFindEstimate().second);
		EXPECT_FALSE(context.isCompletable());
	}

	// endregion

	// region tryFindBestPrevote

	namespace {
		void AssertUnset(const std::pair<model::HeightHashPair, bool>& pair) {
			EXPECT_FALSE(pair.second);
			EXPECT_EQ(model::HeightHashPair(), pair.first);
		}

		void AssertSet(const model::HeightHashPair& expected, const std::pair<model::HeightHashPair, bool>& pair) {
			EXPECT_TRUE(pair.second);
			EXPECT_EQ(expected, pair.first);
		}

		template<typename TAction>
		void RunTryFindBestPrevoteTest(TAction action) {
			// Arrange:
			auto hashes1 = test::GenerateRandomDataVector<Hash256>(3);
			auto hashes2 = test::GenerateRandomDataVector<Hash256>(3);

			RoundContext context(1000, 670);
			context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 669);
			context.acceptPrevote(Height(7), hashes2.data(), hashes2.size(), 100);

			// Act + Assert:
			action(context, hashes1);
		}
	}

	TEST(TEST_CLASS, BestPrevoteDoesNotExistWhenAllCandidatesHaveFewerThanThresholdPrevotes) {
		// Arrange: votes { 669, 669, 669 }
		RunTryFindBestPrevoteTest([](const auto& context, const auto&) {
			// Act + Assert:
			AssertUnset(context.tryFindBestPrevote());

			// Sanity:
			AssertUnset(context.tryFindBestPrecommit());
		});
	}

	TEST(TEST_CLASS, BestPrevoteExistsWhenAnyCandidateHasExactlyThresholdPrevotes) {
		// Arrange: votes { 670, *670*, 669 }
		RunTryFindBestPrevoteTest([](auto& context, const auto& hashes) {
			context.acceptPrevote(Height(7), hashes.data(), 2, 1);

			// Act + Assert:
			AssertSet(model::HeightHashPair({ Height(8), hashes[1] }), context.tryFindBestPrevote());

			// Sanity:
			AssertUnset(context.tryFindBestPrecommit());
		});
	}

	TEST(TEST_CLASS, BestPrevoteExistsWhenAnyCandidateHasGreaterThanThresholdPrevotes) {
		// Arrange: votes { 800, *700*, 669 }
		RunTryFindBestPrevoteTest([](auto& context, const auto& hashes) {
			context.acceptPrevote(Height(7), hashes.data(), 2, 31);
			context.acceptPrevote(Height(7), hashes.data(), 1, 100);

			// Act + Assert:
			AssertSet(model::HeightHashPair({ Height(8), hashes[1] }), context.tryFindBestPrevote());

			// Sanity:
			AssertUnset(context.tryFindBestPrecommit());
		});
	}

	// endregion

	// region tryFindBestPrecommit

	namespace {
		template<typename TAction>
		void RunTryFindBestPrecommitTest(TAction action) {
			// Arrange:
			auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
			auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);

			RoundContext context(1000, 670);
			context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 669);
			context.acceptPrevote(Height(7), hashes2.data(), hashes2.size(), 100);

			context.acceptPrecommit(Height(9), hashes1[2], 669);
			context.acceptPrecommit(Height(9), hashes2[2], 150);

			// Act + Assert:
			action(context, hashes1);
		}
	}

	TEST(TEST_CLASS, BestPrecommitDoesNotExistWhenAllCandidatesHaveFewerThanThresholdPrevotesAndPrecommits) {
		// Arrange: votes { 669, 669, 669, 669 }, commits { 669, 669, 669, 0 }
		RunTryFindBestPrecommitTest([](const auto& context, const auto&) {
			// Act + Assert:
			AssertUnset(context.tryFindBestPrecommit());

			// Sanity:
			AssertUnset(context.tryFindBestPrevote());
		});
	}

	TEST(TEST_CLASS, BestPrecommitDoesNotExistWhenAllCandidatesHaveFewerThanThresholdPrevotesButNotPrecommits) {
		// Arrange: votes { 669, 669, 669, 669 }, commits { 769, 769, 769, 0 }
		RunTryFindBestPrecommitTest([](auto& context, const auto& hashes) {
			context.acceptPrecommit(Height(9), hashes[2], 100);

			// Act + Assert:
			AssertUnset(context.tryFindBestPrecommit());

			// Sanity:
			AssertUnset(context.tryFindBestPrevote());
		});
	}

	TEST(TEST_CLASS, BestPrecommitDoesNotExistWhenAllCandidatesHaveFewerThanThresholdPrecommitsButNotPrevotes) {
		// Arrange: votes { 769, 769, 769, *769* }, commits { 669, 669, 669, 0 }
		RunTryFindBestPrecommitTest([](auto& context, const auto& hashes) {
			context.acceptPrevote(Height(7), hashes.data(), hashes.size(), 100);

			// Act + Assert:
			AssertUnset(context.tryFindBestPrecommit());

			// Sanity:
			AssertSet(model::HeightHashPair({ Height(10), hashes[3] }), context.tryFindBestPrevote());
		});
	}

	TEST(TEST_CLASS, BestPrecommitExistsWhenAnyNodeHasExactlyThresholdPrevotesAndPrecommits) {
		// Arrange: votes { 670, 670, *670*, 669 }, commits { 670, *670*, 669, 0 }
		RunTryFindBestPrecommitTest([](auto& context, const auto& hashes) {
			context.acceptPrevote(Height(7), hashes.data(), 3, 1);
			context.acceptPrecommit(Height(8), hashes[1], 1);

			// Act + Assert:
			AssertSet(model::HeightHashPair({ Height(8), hashes[1] }), context.tryFindBestPrecommit());

			// Sanity:
			AssertSet(model::HeightHashPair({ Height(9), hashes[2] }), context.tryFindBestPrevote());
		});
	}

	TEST(TEST_CLASS, BestPrecommitExistsWhenAnyNodeHasGreaterThanThresholdPrevotesAndPrecommits) {
		// Arrange: votes { 800, 800, *700*, 669 }, commits { 790, *690*, 669, 0 }
		RunTryFindBestPrecommitTest([](auto& context, const auto& hashes) {
			context.acceptPrevote(Height(7), hashes.data(), 3, 31);
			context.acceptPrevote(Height(7), hashes.data(), 2, 100);

			context.acceptPrecommit(Height(8), hashes[1], 21);
			context.acceptPrecommit(Height(7), hashes[0], 100);

			// Act + Assert:
			AssertSet(model::HeightHashPair({ Height(8), hashes[1] }), context.tryFindBestPrecommit());

			// Sanity:
			AssertSet(model::HeightHashPair({ Height(9), hashes[2] }), context.tryFindBestPrevote());
		});
	}

	TEST(TEST_CLASS, BestPrecommitDoesNotExistWhenPrevotesAndPrecommitsAreUnrelated) {
		// Arrange: votes { 769, *769*, 669 }, commits { 750 [unknown] }
		RunTryFindBestPrevoteTest([](auto& context, const auto& hashes) {
			context.acceptPrevote(Height(7), hashes.data(), 2, 100);

			context.acceptPrecommit(Height(9), test::GenerateRandomByteArray<Hash256>(), 750);
			context.acceptPrecommit(Height(9), test::GenerateRandomByteArray<Hash256>(), 150);

			// Act + Assert:
			AssertUnset(context.tryFindBestPrecommit());

			// Sanity:
			AssertSet(model::HeightHashPair({ Height(8), hashes[1] }), context.tryFindBestPrevote());
		});
	}

	// endregion

	// region tryFindEstimate

	namespace {
		template<typename TAction>
		void RunTryFindEstimateTest(uint64_t seedWeight, TAction action) {
			// Arrange:
			//  7 - 8 - 9 - 10      | (hashes1)
			//         |||          |
			//          9 - 10 - 11 | (hashes2)
			//  7 - 8 - 9 - 10 - 11 | (hashes3)
			auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
			auto hashes2 = test::GenerateRandomDataVector<Hash256>(3);
			auto hashes3 = test::GenerateRandomDataVector<Hash256>(5);
			hashes2[0] = hashes1[2];

			RoundContext context(1000, 670);
			context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), seedWeight);
			context.acceptPrevote(Height(9), hashes2.data(), hashes2.size(), seedWeight);
			context.acceptPrevote(Height(7), hashes3.data(), hashes3.size(), seedWeight);

			// Act + Assert:
			action(context, hashes1, hashes2, hashes3);
		}
	}

	TEST(TEST_CLASS, EstimateDoesNotExistWhenThereIsNoBestPrevote) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrecommit(Height(9), hashes1[2], 700);

			// Act + Assert:
			AssertUnset(context.tryFindEstimate());

			// Sanity:
			EXPECT_FALSE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateDoesNotExistWhenBestPrevoteCannotReachPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto& hashes2, const auto& hashes3) {
			context.acceptPrevote(Height(7), hashes1.data(), 4, 700);
			context.acceptPrecommit(Height(9), hashes2[0], 350);
			context.acceptPrecommit(Height(9), hashes3[2], 350);

			// Act + Assert:
			AssertUnset(context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateExistsWhenBestPrevoteCanReachPrecommitThreshold_AbovePrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 700);

			// Act + Assert:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateExistsWhenBestPrevoteCanReachPrecommitThreshold_BelowPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 400);

			// Act + Assert:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateExistsWhenBestPrevoteAncestorCanReachPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(8), hashes1[1], 400);

			// Act + Assert:
			AssertSet({ Height(8), hashes1[1] }, context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateExistsWhenBestPrevoteAndUnlinkedCandidateCanReachPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto& hashes3) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 200);
			context.acceptPrecommit(Height(11), hashes3[4], 200);

			// Act + Assert:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateIgnoresPendingCommits) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 400);

			auto unknownHash = test::GenerateRandomByteArray<Hash256>();
			context.acceptPrecommit(Height(10), unknownHash, 500);

			// Act + Assert: pending commit votes are ignored in order to simplify calculation
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	TEST(TEST_CLASS, EstimateCanTransitionFromSetToUnsetWithAdditionalVotes) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 400);

			auto unknownHash = test::GenerateRandomByteArray<Hash256>();
			context.acceptPrecommit(Height(10), unknownHash, 500);

			// Sanity: pending commit votes are ignored in order to simplify calculation
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
			EXPECT_TRUE(context.isCompletable());

			context.acceptPrevote(Height(10), &unknownHash, 1, 10);

			// Act + Assert:
			AssertUnset(context.tryFindEstimate());

			// Sanity:
			EXPECT_TRUE(context.isCompletable());
		});
	}

	// endregion

	// region isDescendant

	TEST(TEST_CLASS, IsDescendantDelegatesToFinalizationHashTree) {
		// Arrange:
		//  7 - 8 - 9 - 10      |
		//     ||| |||          |
		//      8   9 - 10 - 11 |
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);
		hashes2[0] = hashes1[1];
		hashes2[1] = hashes1[2];

		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Act + Assert:
		EXPECT_FALSE(context.isDescendant({ Height(11), hashes2[3] }, { Height(9), hashes1[2] })); // ancestor
		EXPECT_TRUE(context.isDescendant({ Height(9), hashes1[2] }, { Height(9), hashes1[2] })); // self
		EXPECT_TRUE(context.isDescendant({ Height(9), hashes1[2] }, { Height(11), hashes2[3] })); // descendant

		EXPECT_FALSE(context.isDescendant({ Height(9), hashes1[0] }, { Height(11), hashes2[3] })); // unknown parent
		EXPECT_FALSE(context.isDescendant({ Height(9), hashes1[2] }, { Height(11), hashes2[0] })); // unknown child
	}

	// endregion

	// region isCompletable

	TEST(TEST_CLASS, NotCompletableWhenThereIsNoBestPrevote) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrecommit(Height(9), hashes1[2], 700);

			// Act + Assert:
			EXPECT_FALSE(context.isCompletable());

			// Sanity:
			AssertUnset(context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, CompletableWhenEstimateIsAncestorOfBestPrevote) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(8), hashes1[1], 400);

			// Act + Assert:
			EXPECT_TRUE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(8), hashes1[1] }, context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, CompletableWhenBestPrevoteAndUnlinkedCandidateCanReachPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto& hashes3) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 200);
			context.acceptPrecommit(Height(9), hashes3[2], 200);

			// Act + Assert:
			EXPECT_TRUE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, NotCompletableWhenUnseenBestPrevoteDescendantCanReachPrecommitThreshold) {
		// Arrange: assume unseen candidate has 800 votes
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 200);

			// Act + Assert:
			EXPECT_FALSE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, CompletableWhenNoneOfBestPrevoteDescendantsCanReachPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(9), hashes1[2], 400);

			// Act + Assert:
			EXPECT_TRUE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, NotCompletableWhenAnyBestPrevoteDescendantCanReachPrecommitThreshold) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(10), hashes1[3], 400);

			// Act + Assert:
			EXPECT_FALSE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, CompletableIgnoresPendingCommits) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(10), hashes1[3], 400);

			auto unknownHash = test::GenerateRandomByteArray<Hash256>();
			context.acceptPrecommit(Height(10), unknownHash, 500);

			// Act + Assert: pending commit votes are ignored in order to simplify calculation
			EXPECT_FALSE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
		});
	}

	TEST(TEST_CLASS, CompletableCanTransitionFromFalseToTrueWithAdditionalVotes) {
		// Arrange:
		RunTryFindEstimateTest(10, [](auto& context, const auto& hashes1, const auto&, const auto&) {
			context.acceptPrevote(Height(7), hashes1.data(), 3, 700);
			context.acceptPrecommit(Height(10), hashes1[3], 400);

			auto unknownHash = test::GenerateRandomByteArray<Hash256>();
			context.acceptPrecommit(Height(10), unknownHash, 500);

			// Sanity: pending commit votes are ignored in order to simplify calculation
			EXPECT_FALSE(context.isCompletable());
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());

			auto newHashes = std::vector<Hash256>{ hashes1[2], unknownHash };
			context.acceptPrevote(Height(9), newHashes.data(), newHashes.size(), 10);

			// Act + Assert:
			EXPECT_TRUE(context.isCompletable());

			// Sanity:
			AssertSet({ Height(9), hashes1[2] }, context.tryFindEstimate());
		});
	}

	// endregion

	// region weights

	TEST(TEST_CLASS, WeightsReturnsZeroValuesForUnknownCandidate) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);
		auto otherHash = test::GenerateRandomByteArray<Hash256>();

		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes.data(), hashes.size(), 250);

		// Sanity:
		EXPECT_EQ(3u, context.size());

		// Act + Assert:
		EXPECT_EQ(RoundContext::Weights(), context.weights({ Height(11), hashes[0] })); // wrong height
		EXPECT_EQ(RoundContext::Weights(), context.weights({ Height(8), otherHash })); // wrong hash
		EXPECT_EQ(RoundContext::Weights(), context.weights({ Height(8), hashes[0] })); // wrong height or hash
		EXPECT_EQ(RoundContext::Weights(), context.weights({ Height(11), otherHash })); // wrong height and hash
	}

	TEST(TEST_CLASS, WeightsReturnsNonzeroValuesForKnownCandidate) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);

		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes.data(), hashes.size(), 250);

		// Sanity:
		EXPECT_EQ(3u, context.size());

		// Act + Assert:
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(7), hashes[0] }));
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(8), hashes[1] }));
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(9), hashes[2] }));
	}

	// endregion

	// region prevote

	TEST(TEST_CLASS, CanPrevoteNonOverlappingBranches) {
		// Arrange:
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(3);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);

		// Act:
		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Assert:
		EXPECT_EQ(7u, context.size());
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(7), hashes1[0] }));
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(8), hashes1[1] }));
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(9), hashes1[2] }));
		EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(8), hashes2[0] }));
		EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(9), hashes2[1] }));
		EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(10), hashes2[2] }));
		EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(11), hashes2[3] }));
	}

	TEST(TEST_CLASS, CanPrevoteOverlappingBranches) {
		// Arrange:
		//  7 - 8 - 9 - 10      |
		//     ||| |||          |
		//      8   9 - 10 - 11 |
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);
		hashes2[0] = hashes1[1];
		hashes2[1] = hashes1[2];

		// Act:
		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Assert:
		EXPECT_EQ(6u, context.size());
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(7), hashes1[0] }));
		EXPECT_EQ(RoundContext::Weights({ 450, 0 }), context.weights({ Height(8), hashes1[1] }));
		EXPECT_EQ(RoundContext::Weights({ 450, 0 }), context.weights({ Height(9), hashes1[2] }));
		EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(10), hashes1[3] }));
		EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(10), hashes2[2] }));
		EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(11), hashes2[3] }));
	}

	// endregion

	// region precommit

	namespace {
		void AssertPrecommitNonOverlappingBranches(
				const RoundContext& context,
				const std::vector<Hash256>& hashes1,
				const std::vector<Hash256>& hashes2) {
			EXPECT_EQ(7u, context.size());
			EXPECT_EQ(RoundContext::Weights({ 250, 100 }), context.weights({ Height(7), hashes1[0] }));
			EXPECT_EQ(RoundContext::Weights({ 250, 100 }), context.weights({ Height(8), hashes1[1] }));
			EXPECT_EQ(RoundContext::Weights({ 250, 0 }), context.weights({ Height(9), hashes1[2] }));
			EXPECT_EQ(RoundContext::Weights({ 200, 150 }), context.weights({ Height(8), hashes2[0] }));
			EXPECT_EQ(RoundContext::Weights({ 200, 150 }), context.weights({ Height(9), hashes2[1] }));
			EXPECT_EQ(RoundContext::Weights({ 200, 150 }), context.weights({ Height(10), hashes2[2] }));
			EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(11), hashes2[3] }));
		}

		void AssertPrecommitOverlappingBranches(
				const RoundContext& context,
				const std::vector<Hash256>& hashes1,
				const std::vector<Hash256>& hashes2) {
			EXPECT_EQ(6u, context.size());
			EXPECT_EQ(RoundContext::Weights({ 250, 300 }), context.weights({ Height(7), hashes1[0] }));
			EXPECT_EQ(RoundContext::Weights({ 450, 300 }), context.weights({ Height(8), hashes1[1] }));
			EXPECT_EQ(RoundContext::Weights({ 450, 250 }), context.weights({ Height(9), hashes1[2] }));
			EXPECT_EQ(RoundContext::Weights({ 250, 100 }), context.weights({ Height(10), hashes1[3] }));
			EXPECT_EQ(RoundContext::Weights({ 200, 150 }), context.weights({ Height(10), hashes2[2] }));
			EXPECT_EQ(RoundContext::Weights({ 200, 0 }), context.weights({ Height(11), hashes2[3] }));
		}
	}

	TEST(TEST_CLASS, CanPrecommitNonOverlappingBranches) {
		// Arrange:
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(3);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);

		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Act:
		context.acceptPrecommit(Height(8), hashes1[1], 100);
		context.acceptPrecommit(Height(10), hashes2[2], 150);

		// Assert:
		AssertPrecommitNonOverlappingBranches(context, hashes1, hashes2);
	}

	TEST(TEST_CLASS, CanPrecommitNonOverlappingBranches_BeforePrevote) {
		// Arrange:
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(3);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);

		RoundContext context(1000, 670);
		context.acceptPrecommit(Height(8), hashes1[1], 100);
		context.acceptPrecommit(Height(10), hashes2[2], 150);

		// Sanity:
		EXPECT_EQ(0u, context.size());

		// Act:
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Assert:
		AssertPrecommitNonOverlappingBranches(context, hashes1, hashes2);
	}

	TEST(TEST_CLASS, CanPrecommitNonOverlappingBranches_MultipleBeforePrevote) {
		// Arrange:
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(3);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);

		RoundContext context(1000, 670);
		context.acceptPrecommit(Height(8), hashes1[1], 75);
		context.acceptPrecommit(Height(8), hashes1[1], 25);
		context.acceptPrecommit(Height(10), hashes2[2], 110);
		context.acceptPrecommit(Height(10), hashes2[2], 40);

		// Sanity:
		EXPECT_EQ(0u, context.size());

		// Act:
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Assert:
		AssertPrecommitNonOverlappingBranches(context, hashes1, hashes2);
	}

	TEST(TEST_CLASS, CanPrecommitOverlappingBranches) {
		// Arrange:
		//  7 - 8 - 9 - 10      |
		//     ||| |||          |
		//      8   9 - 10 - 11 |
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);
		hashes2[0] = hashes1[1];
		hashes2[1] = hashes1[2];

		RoundContext context(1000, 670);
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Act:
		context.acceptPrecommit(Height(10), hashes1[3], 100);
		context.acceptPrecommit(Height(10), hashes2[2], 150);
		context.acceptPrecommit(Height(8), hashes1[1], 50);

		// Assert:
		AssertPrecommitOverlappingBranches(context, hashes1, hashes2);
	}

	TEST(TEST_CLASS, CanPrecommitOverlappingBranches_BeforePrevote) {
		// Arrange:
		//  7 - 8 - 9 - 10      |
		//     ||| |||          |
		//      8   9 - 10 - 11 |
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);
		hashes2[0] = hashes1[1];
		hashes2[1] = hashes1[2];

		RoundContext context(1000, 670);
		context.acceptPrecommit(Height(10), hashes1[3], 100);
		context.acceptPrecommit(Height(10), hashes2[2], 150);
		context.acceptPrecommit(Height(8), hashes1[1], 50);

		// Sanity:
		EXPECT_EQ(0u, context.size());

		// Act:
		context.acceptPrevote(Height(7), hashes1.data(), hashes1.size(), 250);
		context.acceptPrevote(Height(8), hashes2.data(), hashes2.size(), 200);

		// Assert:
		AssertPrecommitOverlappingBranches(context, hashes1, hashes2);
	}

	// endregion
}}
