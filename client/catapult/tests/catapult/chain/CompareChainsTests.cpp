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

	// region chain info

	TEST(TEST_CLASS, RemoteReportedLowerChainScoreIfRemoteChainScoreIsLessThanLocalChainScore) {
		// Arrange:
		MockChainApi local(ChainScore(10), Height(20));
		MockChainApi remote(ChainScore(9), Height(20));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Reported_Lower_Chain_Score, result.Code);
		AssertDefaultChainInformation(result);
	}

	TEST(TEST_CLASS, RemoteReportedEqualChainScoreIfRemoteChainScoreIsEqualToLocalChainScore) {
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

	TEST(TEST_CLASS, RemoteIsTooFarBehindIfRemoteIsMoreThanMaxBlocksToRewriteBehindLocal) {
		// Assert:
		AssertIsTooFarBehind(Height(18), Height(7), 10, true);
	}

	TEST(TEST_CLASS, RemoteIsNotTooFarBehindIfRemoteIsMaxBlocksToRewriteBehindLocal) {
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
		void AssertRemoteReturnedTooManyHashes(uint32_t numHashes, uint32_t analyzeLimit, bool expected) {
			// Arrange:
			MockChainApi local(ChainScore(10), Height(numHashes));
			MockChainApi remote(ChainScore(11), Height(numHashes), numHashes);
			CompareChainsOptions options{ analyzeLimit, 1000 };

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

	TEST(TEST_CLASS, RemoteReturnedTooManyHashesIfItReturnedMoreThanMaxBlocksToAnalyze) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(21, 20, true);
	}

	TEST(TEST_CLASS, RemoteDidNotReturnTooManyHashesIfItReturnedExactlyMaxBlocksToAnalyze) {
		// Assert:
		AssertRemoteReturnedTooManyHashes(20, 20, false);
	}

	TEST(TEST_CLASS, RemoteIsForkedIfTheFirstLocalAndRemoteHashesDoNotMatch) {
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

	TEST(TEST_CLASS, RemoteLiedAboutChainScoreIfLocalIsSameSizeAsRemoteChainAndContainsAllHashesInRemoteChain) {
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

	TEST(TEST_CLASS, RemoteLiedAboutChainScoreIfRemoteChainIsSubsetOfLocalChainButRemoteReportedHigherScore) {
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

	TEST(TEST_CLASS, RemoteIsNotSyncedIfLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { A, B }, Remote { A, B, C }
		auto remoteHashes = test::GenerateRandomHashes(3);
		auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 2);
		MockChainApi local(ChainScore(10), Height(2), localHashes);
		MockChainApi remote(ChainScore(11), Height(3), remoteHashes);
		remote.addBlock(test::GenerateVerifiableBlockAtHeight(Height(3)));

		// Act:
		auto result = CompareChains(local, remote, { 1000, 1000 }).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(2), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
		EXPECT_TRUE(AreChainsConsistent(localHashes.size(), result.CommonBlockHeight, Height(1)));
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedIfLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChainButReturnsMoreHashesThanExpected) {
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
		remote.addBlock(test::GenerateVerifiableBlockAtHeight(Height(3)));

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

			EXPECT_EQ(std::vector<Height>{ expectedHashesFromHeight }, local.hashesFromRequests());
			EXPECT_EQ(std::vector<Height>{ expectedHashesFromHeight }, remote.hashesFromRequests());
		}
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedIfChainsDivergeAndRemoteHasHigherScore) {
		// Assert: note chain height is 20 < 1000
		AssertRemoteIsNotSynced(1000, Height(1), Height(19));
	}

	TEST(TEST_CLASS, CommonBlockHeightIsInfluencedByRewriteLimit) {
		// Assert: note chain height is 20 > 15
		AssertRemoteIsNotSynced(5, Height(15), Height(19));
	}

	// endregion
}}
