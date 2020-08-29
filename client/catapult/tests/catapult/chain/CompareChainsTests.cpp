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
#include "tests/test/core/HashTestUtils.h"
#include "tests/TestHarness.h"

using catapult::model::ChainScore;
using catapult::mocks::MockChainApi;

namespace catapult { namespace chain {

#define TEST_CLASS CompareChainsTests

	namespace {
		// region test utils

		CompareChainsOptions CreateCompareChainsOptions(uint32_t hashesPerBatch, Height::ValueType finalizedHeight) {
			CompareChainsOptions options;
			options.HashesPerBatch = hashesPerBatch;
			options.FinalizedHeightSupplier = [finalizedHeight]() { return Height(finalizedHeight); };
			return options;
		}

		void AssertLocalChainExceptionPropagation(MockChainApi::EntryPoint entryPoint) {
			// Arrange:
			auto commonHashes = test::GenerateRandomHashes(3);
			auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));

			MockChainApi local(ChainScore(10), Height(4));
			local.setHashes(Height(1), localHashes);
			local.setError(entryPoint);

			MockChainApi remote(ChainScore(11), Height(4));
			remote.setHashes(Height(1), remoteHashes);

			// Act + Assert:
			EXPECT_THROW(CompareChains(local, remote, CreateCompareChainsOptions(1000, 1)).get(), catapult_runtime_error);
		}

		void AssertRemoteChainExceptionPropagation(MockChainApi::EntryPoint entryPoint) {
			// Arrange:
			auto commonHashes = test::GenerateRandomHashes(3);
			auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));

			MockChainApi local(ChainScore(10), Height(4));
			local.setHashes(Height(1), localHashes);

			MockChainApi remote(ChainScore(11), Height(4));
			remote.setHashes(Height(1), remoteHashes);
			remote.setError(entryPoint);

			// Act + Assert:
			EXPECT_THROW(CompareChains(local, remote, CreateCompareChainsOptions(1000, 1)).get(), catapult_runtime_error);
		}

		void AssertDefaultChainStatistics(const CompareChainsResult& result) {
			// Assert:
			EXPECT_EQ(Height(static_cast<Height::ValueType>(-1)), result.CommonBlockHeight);
			EXPECT_EQ(0u, result.ForkDepth);
		}

		// endregion
	}

	// region chain statistics

	TEST(TEST_CLASS, RemoteReportedLowerChainScoreWhenRemoteChainScoreIsLessThanLocalChainScore) {
		// Arrange:
		MockChainApi local(ChainScore(10), Height(20));
		MockChainApi remote(ChainScore(9), Height(20));

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(1000, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Reported_Lower_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	TEST(TEST_CLASS, RemoteReportedEqualChainScoreWhenRemoteChainScoreIsEqualToLocalChainScore) {
		// Arrange:
		MockChainApi local(ChainScore(10), Height(20));
		MockChainApi remote(ChainScore(10), Height(20));

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(1000, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Reported_Equal_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	namespace {
		void AssertIsTooFarBehind(Height localHeight, Height remoteHeight, Height localFinalizedHeight, bool isErrorExpected) {
			// Arrange:
			MockChainApi local(ChainScore(10), localHeight);
			local.setHashes(localFinalizedHeight, model::HashRange());

			MockChainApi remote(ChainScore(11), remoteHeight);
			remote.setHashes(localFinalizedHeight, model::HashRange());

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(1000, localFinalizedHeight.unwrap())).get();

			// Assert:
			if (isErrorExpected)
				EXPECT_EQ(ChainComparisonCode::Remote_Is_Too_Far_Behind, result.Code);
			else
				EXPECT_NE(ChainComparisonCode::Remote_Is_Too_Far_Behind, result.Code);

			AssertDefaultChainStatistics(result);
		}
	}

	TEST(TEST_CLASS, RemoteIsTooFarBehindWhenRemoteHeightIsLessThanOrEqualToLocalFinalizedHeight) {
		AssertIsTooFarBehind(Height(18), Height(2), Height(7), true);
		AssertIsTooFarBehind(Height(18), Height(6), Height(7), true);
		AssertIsTooFarBehind(Height(18), Height(7), Height(7), true);
	}

	TEST(TEST_CLASS, RemoteIsNotTooFarBehindWhenRemoteHeightIsGreaterThanLocalFinalizedHeight) {
		AssertIsTooFarBehind(Height(18), Height(8), Height(7), false);
		AssertIsTooFarBehind(Height(18), Height(200), Height(7), false);
	}

	TEST(TEST_CLASS, LocalChainStatisticsExceptionIsPropagated) {
		AssertLocalChainExceptionPropagation(MockChainApi::EntryPoint::Chain_Statistics);
	}

	TEST(TEST_CLASS, RemoteChainStatisticsExceptionIsPropagated) {
		AssertRemoteChainExceptionPropagation(MockChainApi::EntryPoint::Chain_Statistics);
	}

	// endregion

	// region hash traits

	namespace {
		auto GenerateDeterministicHashes(uint32_t count, uint8_t seed = 1) {
			auto hashes = test::GenerateRandomHashes(count);

			auto i = 0u;
			for (auto& hash : hashes)
				hash = { { static_cast<uint8_t>(i++ * seed) } };

			return hashes;
		}

		struct OneBatchTraits {
			static constexpr uint32_t Prepare_Adjustment = 0;

			static void PrepareTooManyHashes(MockChainApi& chainApi, uint32_t numHashes, uint32_t) {
				chainApi.setHashes(Height(1), test::GenerateRandomHashes(numHashes));
			}

			static size_t Prepare(MockChainApi& chainApi, const model::HashRange& hashes, uint32_t, Height height = Height(1)) {
				chainApi.setHashes(height, hashes);
				return hashes.size();
			}
		};

		struct MultiBatchTraits {
			static constexpr uint32_t Prepare_Adjustment = 40;

			static void PrepareTooManyHashes(MockChainApi& chainApi, uint32_t numHashes, uint32_t hashesPerBatch) {
				chainApi.setHashes(Height(1), GenerateDeterministicHashes(hashesPerBatch, 1));
				chainApi.setHashes(Height(1 + hashesPerBatch), GenerateDeterministicHashes(numHashes, 2));
				chainApi.setHashes(Height(1 + 2 * hashesPerBatch), GenerateDeterministicHashes(hashesPerBatch, 3));
			}

			static size_t Prepare(
					MockChainApi& chainApi,
					const model::HashRange& hashes,
					uint32_t hashesPerBatch,
					Height height = Height(1)) {
				chainApi.setHashes(height, GenerateDeterministicHashes(hashesPerBatch, 1));
				chainApi.setHashes(height + Height(hashesPerBatch), GenerateDeterministicHashes(hashesPerBatch, 2));
				chainApi.setHashes(height + Height(2 * hashesPerBatch), hashes);
				return 2 * hashesPerBatch + hashes.size();
			}
		};
	}

#define HASH_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_OneBatch) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OneBatchTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultiBatch) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultiBatchTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region hash - common

	namespace {
		template<typename TTraits>
		void AssertRemoteReturnedTooManyHashes(uint32_t numHashes, uint32_t hashesPerBatch, bool isErrorExpected) {
			// Arrange:
			MockChainApi local(ChainScore(10), Height(3 * hashesPerBatch));
			TTraits::PrepareTooManyHashes(local, hashesPerBatch, hashesPerBatch);

			MockChainApi remote(ChainScore(11), Height(2 * hashesPerBatch + numHashes));
			TTraits::PrepareTooManyHashes(remote, numHashes, hashesPerBatch);

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(hashesPerBatch, 1)).get();

			// Assert:
			if (isErrorExpected)
				EXPECT_EQ(ChainComparisonCode::Remote_Returned_Too_Many_Hashes, result.Code);
			else
				EXPECT_NE(ChainComparisonCode::Remote_Returned_Too_Many_Hashes, result.Code);

			AssertDefaultChainStatistics(result);
		}
	}

	HASH_TEST(RemoteReturnedTooManyHashesWhenItReturnsMoreThanHashesPerBatch) {
		AssertRemoteReturnedTooManyHashes<TTraits>(21, 20, true);
		AssertRemoteReturnedTooManyHashes<TTraits>(100, 20, true);
	}

	HASH_TEST(RemoteDidNotReturnTooManyHashesWhenItReturnsLessThanHashesPerBatch) {
		AssertRemoteReturnedTooManyHashes<TTraits>(19, 20, false);
		AssertRemoteReturnedTooManyHashes<TTraits>(10, 20, false);
	}

	HASH_TEST(RemoteLiedAboutChainScoreWhenLocalIsSameSizeAsRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { ... A, B, C }, Remote { ... A, B, C }
		auto adjustment = TTraits::Prepare_Adjustment;
		auto commonHashes = test::GenerateRandomHashes(3);

		MockChainApi local(ChainScore(10), Height(adjustment + 3));
		TTraits::Prepare(local, commonHashes, 20);

		MockChainApi remote(ChainScore(11), Height(adjustment + 3));
		TTraits::Prepare(remote, commonHashes, 20);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteLiedAboutChainScoreWhenRemoteChainIsSubsetOfLocalChainButRemoteReportedHigherScore) {
		// Arrange: Local { ... A, B, C }, Remote { ... A, B }
		auto adjustment = TTraits::Prepare_Adjustment;
		auto localHashes = test::GenerateRandomHashes(3);
		auto remoteHashes = test::GenerateRandomHashesSubset(localHashes, 2);

		MockChainApi local(ChainScore(10), Height(adjustment + 3));
		TTraits::Prepare(local, localHashes, 20);

		MockChainApi remote(ChainScore(11), Height(adjustment + 2));
		TTraits::Prepare(remote, remoteHashes, 20);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteIsNotPunishedWhenLocalHeightIncreasesDuringChainCompare) {
		// Arrange: simulate the scenario where the local height increases in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 13 local hashes
		// - 3. return X + 13 remote hashes so that the remote still has a better score
		auto adjustment = TTraits::Prepare_Adjustment;
		auto commonHashes = test::GenerateRandomHashes(13);

		MockChainApi local(ChainScore(10), Height(adjustment + 8));
		TTraits::Prepare(local, commonHashes, 20);

		MockChainApi remote(ChainScore(11), Height(adjustment + 13));
		TTraits::Prepare(remote, commonHashes, 20);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert: remote was not punished
		EXPECT_EQ(ChainComparisonCode::Local_Height_Updated, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { ... A, B }, Remote { ... A, B, C }
		auto adjustment = TTraits::Prepare_Adjustment;
		auto remoteHashes = test::GenerateRandomHashes(3);
		auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 2);

		MockChainApi local(ChainScore(10), Height(adjustment + 2));
		auto numLocalHashes = TTraits::Prepare(local, localHashes, 20);

		MockChainApi remote(ChainScore(11), Height(adjustment + 3));
		TTraits::Prepare(remote, remoteHashes, 20);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(numLocalHashes), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChainButReturnsMoreHashesThanExpected) {
		// Arrange: simulate the scenario where the local height increases in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 12 local hashes
		// - 3. return X + 13 remote hashes so that the remote still has a better score
		auto adjustment = TTraits::Prepare_Adjustment;
		auto remoteHashes = test::GenerateRandomHashes(13);
		auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 12);

		MockChainApi local(ChainScore(10), Height(adjustment + 8));
		auto numLocalHashes = TTraits::Prepare(local, localHashes, 20);

		MockChainApi remote(ChainScore(11), Height(adjustment + 13));
		TTraits::Prepare(remote, remoteHashes, 20);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(numLocalHashes), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChainButReturnsFewerHashesThanExpected) {
		// Arrange: simulate the scenario where the local height decreases in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 6 local hashes
		// - 3. return X + 13 remote hashes so that the remote still has a better score
		auto adjustment = TTraits::Prepare_Adjustment;
		auto remoteHashes = test::GenerateRandomHashes(13);
		auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 6);

		MockChainApi local(ChainScore(10), Height(adjustment + 8));
		auto numLocalHashes = TTraits::Prepare(local, localHashes, 20);

		MockChainApi remote(ChainScore(11), Height(adjustment + 13));
		TTraits::Prepare(remote, remoteHashes, 20);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(numLocalHashes), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
	}

	namespace {
		template<typename TTraits>
		void AssertRemoteIsNotSynced(
				Height localFinalizedHeight,
				uint32_t adjustment,
				const std::vector<std::pair<Height, uint32_t>>& expectedHashesFromRequests) {
			// Arrange: Local { ..., A, B, C, D }, Remote { ..., A, B, C, E }
			auto commonHashes = test::GenerateRandomHashes(10 - localFinalizedHeight.unwrap());
			auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
			auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));

			MockChainApi local(ChainScore(10), localFinalizedHeight + Height(adjustment + 11));
			TTraits::Prepare(local, localHashes, 20, localFinalizedHeight);

			MockChainApi remote(ChainScore(11), localFinalizedHeight + Height(adjustment + 11));
			TTraits::Prepare(remote, remoteHashes, 20, localFinalizedHeight);

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, localFinalizedHeight.unwrap())).get();

			// Assert:
			EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
			EXPECT_EQ(Height(adjustment + 9), result.CommonBlockHeight);
			EXPECT_EQ(1u, result.ForkDepth);

			// - check requests
			EXPECT_EQ(expectedHashesFromRequests, local.hashesFromRequests());
			EXPECT_EQ(expectedHashesFromRequests, remote.hashesFromRequests());
		}
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScore_OneBatch) {
		AssertRemoteIsNotSynced<OneBatchTraits>(Height(1), 0, { { Height(1), 20 } });
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScoreAndLastFinalizedBlockIsNotNemesis_OneBatch) {
		AssertRemoteIsNotSynced<OneBatchTraits>(Height(7), 0, { { Height(7), 20 } });
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScore_MultiBatch) {
		AssertRemoteIsNotSynced<MultiBatchTraits>(Height(1), 40, { { Height(1), 20 }, { Height(21), 20 }, { Height(41), 20 } });
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScoreAndLastFinalizedBlockIsNotNemesis_MultiBatch) {
		AssertRemoteIsNotSynced<MultiBatchTraits>(Height(7), 40, { { Height(7), 20 }, { Height(27), 20 }, { Height(47), 20 } });
	}

	// endregion

	// region hash (one batch)

	TEST(TEST_CLASS, RemoteIsForkedWhenTheFirstLocalAndRemoteHashesDoNotMatch) {
		// Arrange: Local { A, B, C }, Remote { D, B, C }
		auto localHashes = test::GenerateRandomHashes(3);
		auto remoteHashes = test::GenerateRandomHashes(3);
		*--remoteHashes.end() = *--localHashes.cend();
		*----remoteHashes.end() = *----localHashes.cend();

		MockChainApi local(ChainScore(10), Height(3));
		local.setHashes(Height(1), localHashes);

		MockChainApi remote(ChainScore(11), Height(3));
		remote.setHashes(Height(1), remoteHashes);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(1000, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Forked, result.Code);
		AssertDefaultChainStatistics(result);
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenLocalAndRemoteContainExactlyOneBatchWithDifferentLastHash) {
		// Arrange: Local { ..., A, B, C, D || }, Remote { ..., A, B, C, E || }; where || indicates end of batch
		auto commonHashes = test::GenerateRandomHashes(19);
		auto localHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));
		auto remoteHashes = test::ConcatHashes(commonHashes, test::GenerateRandomHashes(1));

		MockChainApi local(ChainScore(10), Height(3 + 19));
		local.setHashes(Height(3), localHashes);

		MockChainApi remote(ChainScore(11), Height(3 + 19));
		remote.setHashes(Height(3), remoteHashes);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 3)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(21), result.CommonBlockHeight);
		EXPECT_EQ(1u, result.ForkDepth);

		// - check requests
		auto expectedHashesFromRequests = std::vector<std::pair<Height, uint32_t>>{ { Height(3), 20 } };
		EXPECT_EQ(expectedHashesFromRequests, local.hashesFromRequests());
		EXPECT_EQ(expectedHashesFromRequests, remote.hashesFromRequests());
	}

	TEST(TEST_CLASS, LocalHashesFromExceptionIsPropagated) {
		AssertLocalChainExceptionPropagation(MockChainApi::EntryPoint::Hashes_From);
	}

	TEST(TEST_CLASS, RemoteHashesFromExceptionIsPropagated) {
		AssertRemoteChainExceptionPropagation(MockChainApi::EntryPoint::Hashes_From);
	}

	// endregion

	// region hash (multiple batches)

	namespace {
		void AssertRemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchButRemoteHasMultipleBatches(uint64_t adjustment) {
			// Arrange: Local { ..., A, B, C || }, Remote { ..., A, B, C || D }; where || indicates end of batch
			auto commonHashes = test::GenerateRandomHashes(20);
			auto remoteHashes = test::GenerateRandomHashes(1);

			MockChainApi local(ChainScore(10), Height(3 + 19 - adjustment));
			local.setHashes(Height(3), commonHashes);

			MockChainApi remote(ChainScore(11), Height(3 + 20));
			remote.setHashes(Height(3), commonHashes);

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 3)).get();

			// Assert:
			EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
			EXPECT_EQ(Height(22), result.CommonBlockHeight);
			EXPECT_EQ(0u, result.ForkDepth);

			// - check requests
			auto expectedHashesFromRequests = std::vector<std::pair<Height, uint32_t>>{ { Height(3), 20 } };
			EXPECT_EQ(expectedHashesFromRequests, local.hashesFromRequests());
			EXPECT_EQ(expectedHashesFromRequests, remote.hashesFromRequests());
		}
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchButRemoteHasMultipleBatches) {
		AssertRemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchButRemoteHasMultipleBatches(0);
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchWithMisreportedSizeButRemoteHasMultipleBatches) {
		AssertRemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchButRemoteHasMultipleBatches(1);
	}

	TEST(TEST_CLASS, RemoteLiedAboutChainScoreWhenSecondBatchContainsUnexpectedNumberofHashes) {
		// Arrange: remote returns too few hashes in the second batch
		auto commonHashes1 = test::GenerateRandomHashes(20);
		auto commonHashes2 = test::GenerateRandomHashes(20);
		auto commonHashes3 = test::GenerateRandomHashes(20);

		auto localHashes3 = test::GenerateRandomHashesSubset(commonHashes3, 19);
		auto remoteHashes2 = test::GenerateRandomHashesSubset(commonHashes2, 19);

		MockChainApi local(ChainScore(10), Height(3 + 59));
		local.setHashes(Height(3), commonHashes1);
		local.setHashes(Height(23), commonHashes2);
		local.setHashes(Height(43), localHashes3);

		MockChainApi remote(ChainScore(11), Height(3 + 59));
		remote.setHashes(Height(3), commonHashes1);
		remote.setHashes(Height(23), remoteHashes2);
		remote.setHashes(Height(43), commonHashes3);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 3)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	// endregion
}}
