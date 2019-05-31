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

#include "catapult/chain/CompareChains.h"
#include "catapult/model/BlockUtils.h"
#include "tests/catapult/chain/test/MockChainApi.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/TestHarness.h"
#include <map>

using catapult::model::ChainScore;
using catapult::mocks::MockChainApi;

namespace catapult { namespace chain {

#define TEST_CLASS CompareChainsTests

	// region CalculateMaxHashesToAnalyze

	namespace {
		void AssertMaxHashesToAnalyze(uint32_t expected, uint32_t maxBlocksToAnalyze, uint32_t maxBlocksToRewrite) {
			// Act:
			auto result = CalculateMaxHashesToAnalyze({ maxBlocksToAnalyze, maxBlocksToRewrite });

			// Assert:
			EXPECT_EQ(expected, result);
		}
	}

	TEST(TEST_CLASS, CalculateMaxHashesToAnalyzeReturnsMaxBlocksWhenLargerThanRewriteLimitPlusTwo) {
		AssertMaxHashesToAnalyze(20, 20, 10);
		AssertMaxHashesToAnalyze(20, 20, 15);
		AssertMaxHashesToAnalyze(20, 20, 18);
	}

	TEST(TEST_CLASS, CalculateMaxHashesToAnalyzeReturnsMaxRewriteLimitPlusTwoWhenLargerThanMaxBlocks) {
		AssertMaxHashesToAnalyze(21, 20, 19);
		AssertMaxHashesToAnalyze(22, 20, 20);
		AssertMaxHashesToAnalyze(32, 20, 30);
	}

	// endregion

	// region chain info

	namespace {
		void AssertLocalChainExceptionPropagation(MockChainApi::EntryPoint entryPoint) {
			// Arrange:
			auto commonHashes = test::GenerateRandomHashes(3);
			auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			MockChainApi local(ChainScore(10), Height(4), localHashes);
			MockChainApi remote(ChainScore(11), Height(4), remoteHashes);
			local.setError(entryPoint);

			// Act + Assert:
			EXPECT_THROW(CompareChains(local, remote, { 1000, 1000 }).get(), catapult_runtime_error);
		}

		void AssertRemoteChainExceptionPropagation(MockChainApi::EntryPoint entryPoint) {
			// Arrange:
			auto commonHashes = test::GenerateRandomHashes(3);
			auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			MockChainApi local(ChainScore(10), Height(4), localHashes);
			MockChainApi remote(ChainScore(11), Height(4), remoteHashes);
			remote.setError(entryPoint);

			// Act + Assert:
			EXPECT_THROW(CompareChains(local, remote, { 1000, 1000 }).get(), catapult_runtime_error);
		}

		void AssertDefaultChainInformation(const CompareChainsResult& result) {
			// Assert:
			EXPECT_EQ(Height(static_cast<Height::ValueType>(-1)), result.CommonBlockHeight);
			EXPECT_EQ(0u, result.ForkDepth);
		}

		bool AreChainsConsistent(size_t numLocalHashes, Height commonBlockHeight, Height hashesFromHeight) {
			// if consistent, the common block height will be equal to the local chain height
			// note that the common block height needs to be adjusted by the height of the hashes from request
			// in order to get the difference index relative to the local hashes
			auto differenceIndex = (commonBlockHeight - hashesFromHeight + Height(1)).unwrap();
			CATAPULT_LOG(debug) << "numLocalHashes = " << numLocalHashes << ", differenceIndex = " << differenceIndex;
			return numLocalHashes == differenceIndex;
		}
	}

	TEST(TEST_CLASS, LocalChainScoreZeroBypassesSync) {
		// Arrange:
		MockChainApi local(ChainScore(0), Height(20));
		MockChainApi remote(ChainScore(9), Height(20));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Local_Chain_Score_Zero, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, RemoteReportedLowerChainScoreWhenRemoteChainScoreIsLessThanLocalChainScore) {
		// Arrange:
		MockChainApi local(ChainScore(10), Height(20));
		MockChainApi remote(ChainScore(9), Height(20));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Reported_Lower_Chain_Score, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, RemoteReportedEqualChainScoreWhenRemoteChainScoreIsEqualToLocalChainScore) {
		// Arrange:
		MockChainApi local(ChainScore(10), Height(20));
		MockChainApi remote(ChainScore(10), Height(20));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Reported_Equal_Chain_Score, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, LocalChainInfoExceptionIsPropagated) {
		// Assert:
		AssertLocalChainExceptionPropagation(MockChainApi::EntryPoint::Chain_Info);
	}

	TEST(TEST_CLASS, RemoteChainInfoExceptionIsPropagated) {
		// Assert:
		AssertRemoteChainExceptionPropagation(MockChainApi::EntryPoint::Chain_Info);
	}

	namespace {
		void AssertIsTooFarBehind(Height localHeight, Height remoteHeight, uint32_t rewriteLimit, bool expected) {
			// Arrange:
			MockChainApi local(ChainScore(10), localHeight);
			MockChainApi remote(ChainScore(11), remoteHeight);
			CompareChainsOptions options{ 1000, rewriteLimit };

			// Act:
			auto result = CompareChains(local, remote, options).get();

			// Assert:
			if (expected)
				EXPECT_EQ(ChainComparisonCode::Remote_Is_Too_Far_Behind, result.Code);
			else
				EXPECT_NE(ChainComparisonCode::Remote_Is_Too_Far_Behind, result.Code);

			AssertDefaultChainInformation(result);
		}
	}

	TEST(TEST_CLASS, RemoteIsTooFarBehindWhenRemoteIsMoreThanMaxBlocksToRewriteBehindLocal) {
		// Assert:
		AssertIsTooFarBehind(Height(18), Height(7), 10, true);
	}

	TEST(TEST_CLASS, RemoteIsNotTooFarBehindWhenRemoteIsMaxBlocksToRewriteBehindLocal) {
		// Assert:
		AssertIsTooFarBehind(Height(17), Height(7), 10, false);
	}

	TEST(TEST_CLASS, LocalCanBeMoreThanMaxBlocksToRewriteBehindRemote) {
		// Assert:
		AssertIsTooFarBehind(Height(7), Height(18), 10, false);
	}

	// endregion

	// region hash

	namespace {
		void AssertRemoteReturnedTooManyHashes(uint32_t numHashes, uint32_t analyzeLimit, uint32_t rewriteLimit, bool expected) {
			// Arrange:
			MockChainApi local(ChainScore(10), Height(numHashes));
			MockChainApi remote(ChainScore(11), Height(numHashes), numHashes);
			CompareChainsOptions options{ analyzeLimit, rewriteLimit };

			// Act:
			auto result = CompareChains(local, remote, options).get();

			// Assert:
			if (expected)
				EXPECT_EQ(ChainComparisonCode::Remote_Returned_Too_Many_Hashes, result.Code);
			else
				EXPECT_NE(ChainComparisonCode::Remote_Returned_Too_Many_Hashes, result.Code);

			AssertDefaultChainInformation(result);
		}
	}

	TEST(TEST_CLASS, RemoteReturnedTooManyHashesWhenItReturnedMoreThanMaxBlocksToAnalyze) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(21, 20, 10, true);
	}

	TEST(TEST_CLASS, RemoteDidNotReturnTooManyHashesWhenItReturnedExactlyMaxBlocksToAnalyze) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(20, 20, 10, false);
	}

	TEST(TEST_CLASS, RemoteDidNotReturnTooManyHashesWhenItReturnedLessThanRewriteLimit) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(21, 20, 30, false);
	}

	TEST(TEST_CLASS, RemoteDidNotReturnTooManyHashesWhenItMatchesRewriteLimit) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(32, 20, 30, false);
	}

	TEST(TEST_CLASS, RemoteReturnedTooManyHashesWhenItReturnedMoreThanRewriteLimit) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(33, 20, 30, true);
	}

	TEST(TEST_CLASS, RemoteIsForkedWhenTheFirstLocalAndRemoteHashesDoNotMatch) {
		// Arrange: Local { A, B, C }, Remote { D, B, C }
		auto localHashes = test::GenerateRandomHashes(3);
		auto remoteHashes = test::GenerateRandomHashes(3);
		*--remoteHashes.end() = *--localHashes.cend();
		*----remoteHashes.end() = *----localHashes.cend();
		MockChainApi local(ChainScore(10), Height(3), localHashes);
		MockChainApi remote(ChainScore(11), Height(3), remoteHashes);

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Forked, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, RemoteLiedAboutChainScoreWhenLocalIsSameSizeAsRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { A, B, C }, Remote { A, B, C }
		auto commonHashes = test::GenerateRandomHashes(3);
		MockChainApi local(ChainScore(10), Height(3), commonHashes);
		MockChainApi remote(ChainScore(11), Height(3), commonHashes);

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, RemoteLiedAboutChainScoreWhenRemoteChainIsSubsetOfLocalChainButRemoteReportedHigherScore) {
		// Arrange: Local { A, B, C }, Remote { A, B }
		auto localHashes = test::GenerateRandomHashes(3);
		auto remoteHashes = test::GenerateRandomHashesSubset(localHashes, 2);
		MockChainApi local(ChainScore(10), Height(3), localHashes);
		MockChainApi remote(ChainScore(11), Height(2), remoteHashes);

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { A, B }, Remote { A, B, C }
		auto remoteHashes = test::GenerateRandomHashes(3);
		auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 2);
		MockChainApi local(ChainScore(10), Height(2), localHashes);
		MockChainApi remote(ChainScore(11), Height(3), remoteHashes);
		remote.addBlock(test::GenerateBlockWithTransactions(0, Height(3)));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(2), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
		EXPECT_TRUE(AreChainsConsistent(localHashes.size(), result.CommonBlockHeight, Height(1)));
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChainButReturnsMoreHashesThanExpected) {
		// Arrange: simulate the scenario where the local height increases in-between the height and hashes requests
		//   1. report local height as 8
		//   2. return 12 local hashes (12 > 8)
		//   3. return 13 remote hashes so that the remote still has a better score
		//   (notice that this is only exploitable when local height < MaxBlocksToRewrite because even if local height grows in-between
		//   (1) and (2) at most MaxBlocksToRewrite will be returned)
		auto remoteHashes = test::GenerateRandomHashes(13);
		auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 12);
		MockChainApi local(ChainScore(10), Height(8), localHashes);
		MockChainApi remote(ChainScore(11), Height(13), remoteHashes);
		remote.addBlock(test::GenerateBlockWithTransactions(0, Height(3)));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert: common block height is greater than initial local height
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(12), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
		EXPECT_TRUE(AreChainsConsistent(localHashes.size(), result.CommonBlockHeight, Height(1)));
	}

	TEST(TEST_CLASS, LocalHashesFromExceptionIsPropagated) {
		// Assert:
		AssertLocalChainExceptionPropagation(MockChainApi::EntryPoint::Hashes_From);
	}

	TEST(TEST_CLASS, RemoteHashesFromExceptionIsPropagated) {
		// Assert:
		AssertRemoteChainExceptionPropagation(MockChainApi::EntryPoint::Hashes_From);
	}

	// endregion

	// region rewrite limit

	namespace {
		void AssertParam(Height expectedHeight, uint32_t expectedNumHashes, const std::vector<std::pair<Height, uint32_t>>& requests) {
			ASSERT_EQ(1u, requests.size());

			const auto& params = requests[0];
			EXPECT_EQ(expectedHeight, params.first);
			EXPECT_EQ(expectedNumHashes, params.second);
		}

		void AssertRemoteIsNotSynced(uint32_t rewriteLimit, Height expectedHashesFromHeight, Height expectedCommonBlockHeight) {
			// Arrange: Local { ..., A, B, C, D }, Remote { ..., A, B, C, E }
			auto commonHashes = test::GenerateRandomHashes(20 - expectedHashesFromHeight.unwrap());
			auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			MockChainApi local(ChainScore(10), Height(20), localHashes);
			MockChainApi remote(ChainScore(11), Height(20), remoteHashes);

			// Act:
			auto result = CompareChains(local, remote, { 1000, rewriteLimit }).get();

			// Assert:
			EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
			EXPECT_EQ(expectedCommonBlockHeight, result.CommonBlockHeight);
			EXPECT_EQ(20u - expectedCommonBlockHeight.unwrap(), result.ForkDepth);
			EXPECT_FALSE(AreChainsConsistent(localHashes.size(), result.CommonBlockHeight, expectedHashesFromHeight));

			auto expectedNumHashes = std::max(1000u, rewriteLimit + 2);
			AssertParam(expectedHashesFromHeight, expectedNumHashes, local.hashesFromRequests());
			AssertParam(expectedHashesFromHeight, expectedNumHashes, remote.hashesFromRequests());
		}
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScore) {
		// Assert: note chain height is 20 < 1000
		AssertRemoteIsNotSynced(1000, Height(1), Height(19));
	}

	TEST(TEST_CLASS, CommonBlockHeightIsInfluencedByRewriteLimit) {
		// Assert: note chain height is 20 > 15
		AssertRemoteIsNotSynced(5, Height(15), Height(19));
	}

	// endregion
}}
