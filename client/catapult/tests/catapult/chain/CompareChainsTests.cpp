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
		// region CompareHashesMockChainApi

		class CompareHashesMockChainApi : public MockChainApi {
		public:
			static ChainScore ChainScoreExceptionTrigger() {
				return ChainScore();
			}

		public:
			CompareHashesMockChainApi(const model::ChainScore& score, Height height)
					: MockChainApi(score, height)
					, m_hashes(test::GenerateRandomDataVector<Hash256>(height.unwrap()))
					, m_maxHashesBatchSize(std::numeric_limits<size_t>::max())
					, m_nextScoreIndex(0)
			{}

		public:
			void syncHashes(const CompareHashesMockChainApi& source, int32_t commonBlockDepth, uint32_t numAddtionalHashes) {
				m_hashes = source.m_hashes;

				for (auto i = commonBlockDepth; i < 0; ++i)
					m_hashes.pop_back();

				for (auto i = 0u; i < numAddtionalHashes; ++i)
					m_hashes.push_back(test::GenerateRandomByteArray<Hash256>());
			}

			void setMaxHashesBatchSize(size_t maxHashesBatchSize) {
				m_maxHashesBatchSize = maxHashesBatchSize;
			}

			void pushChainScore(const ChainScore& score) {
				m_scores.push_back(score);
			}

		private:
			model::ChainScore chainScore() const override {
				if (m_nextScoreIndex >= m_scores.size())
					return MockChainApi::chainScore();

				const auto& score = m_scores[m_nextScoreIndex++];

				// queue exeception for *next* statistics call
				if (m_nextScoreIndex < m_scores.size() && ChainScoreExceptionTrigger() == m_scores[m_nextScoreIndex])
					const_cast<CompareHashesMockChainApi&>(*this).setError(EntryPoint::Chain_Statistics);

				return score;
			}

			std::pair<model::HashRange, bool> lookupHashes(Height height, uint32_t maxHashes) const override {
				if (height.unwrap() > m_hashes.size())
					return std::make_pair(model::HashRange(), false);

				auto startIndex = height.unwrap() - 1;
				auto count = std::min<size_t>({ m_hashes.size() - startIndex, maxHashes, m_maxHashesBatchSize });
				auto hashRange = model::HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(&m_hashes[startIndex]), count);
				return std::make_pair(std::move(hashRange), count);
			}

		public:
			std::vector<Hash256> m_hashes;
			size_t m_maxHashesBatchSize;
			std::vector<ChainScore> m_scores;
			mutable size_t m_nextScoreIndex;
		};

		// endregion

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
		public:
			static constexpr uint32_t Prepare_Adjustment = 0;

		public:
			static void PrepareTooManyHashes(MockChainApi& chainApi, uint32_t numHashes, uint32_t) {
				chainApi.setHashes(Height(1), test::GenerateRandomHashes(numHashes));
			}
		};

		struct MultiBatchTraits {
		public:
			static constexpr uint32_t Prepare_Adjustment = 100;

		public:
			static void PrepareTooManyHashes(MockChainApi& chainApi, uint32_t numHashes, uint32_t hashesPerBatch) {
				chainApi.setHashes(Height(1), GenerateDeterministicHashes(hashesPerBatch, 1));

				auto secondHeight = AverageHeight(1, 3 * hashesPerBatch);
				chainApi.setHashes(secondHeight, GenerateDeterministicHashes(numHashes, 2));

				auto thirdHeight = AverageHeight(secondHeight.unwrap(), 3 * hashesPerBatch);
				chainApi.setHashes(thirdHeight, GenerateDeterministicHashes(hashesPerBatch, 3));
			}

		private:
			static Height AverageHeight(Height::ValueType lhs, Height::ValueType rhs) {
				return Height((lhs + rhs) / 2);
			}
		};
	}

#define HASH_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_OneBatch) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OneBatchTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultiBatch) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultiBatchTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region hash - too many hashes

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

	HASH_TEST(RemoteReturnedTooManyHashesWhenItReturnsZeroHashesPerBatch) {
		AssertRemoteReturnedTooManyHashes<TTraits>(0, 20, true);
	}

	HASH_TEST(RemoteReturnedTooManyHashesWhenItReturnsMoreThanHashesPerBatch) {
		AssertRemoteReturnedTooManyHashes<TTraits>(21, 20, true);
		AssertRemoteReturnedTooManyHashes<TTraits>(100, 20, true);
	}

	HASH_TEST(RemoteDidNotReturnTooManyHashesWhenItReturnsLessThanHashesPerBatch) {
		AssertRemoteReturnedTooManyHashes<TTraits>(19, 20, false);
		AssertRemoteReturnedTooManyHashes<TTraits>(10, 20, false);
	}

	// endregion

	// region hash - basic

	HASH_TEST(RemoteLiedAboutChainScoreWhenLocalIsSameSizeAsRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { ... A, B, C }, Remote { ... A, B, C }
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 3));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 3));
		remote.syncHashes(local, 0, 0);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteLiedAboutChainScoreWhenRemoteChainIsSubsetOfLocalChainButRemoteReportedHigherScore) {
		// Arrange: Local { ... A, B, C }, Remote { ... A, B }
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 3));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 2));
		remote.syncHashes(local, -1, 0);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChain) {
		// Arrange: Local { ... A, B }, Remote { ... A, B, C }
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 2));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 3));
		remote.syncHashes(local, 0, 1);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(adjustment + 2), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
	}

	namespace {
		void AssertRemoteIsNotSynced(
				Height localFinalizedHeight,
				uint32_t adjustment,
				const std::vector<std::pair<Height, uint32_t>>& expectedHashesFromRequests) {
			// Arrange: Local { ..., A, B, C, D }, Remote { ..., A, B, C, E }
			CompareHashesMockChainApi local(ChainScore(10), localFinalizedHeight + Height(adjustment + 11));
			CompareHashesMockChainApi remote(ChainScore(11), localFinalizedHeight + Height(adjustment + 11));
			remote.syncHashes(local, -1, 1);

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, localFinalizedHeight.unwrap())).get();

			// Assert:
			EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
			EXPECT_EQ(localFinalizedHeight + Height(adjustment + 10), result.CommonBlockHeight);
			EXPECT_EQ(1u, result.ForkDepth);

			// - check requests
			EXPECT_EQ(expectedHashesFromRequests, local.hashesFromRequests());
			EXPECT_EQ(expectedHashesFromRequests, remote.hashesFromRequests());
		}
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScore_OneBatch) {
		AssertRemoteIsNotSynced(Height(1), 0, { { Height(1), 20 } });
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScoreAndLastFinalizedBlockIsNotNemesis_OneBatch) {
		AssertRemoteIsNotSynced(Height(7), 0, { { Height(7), 20 } });
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScore_MultiBatch) {
		AssertRemoteIsNotSynced(Height(1), 100, {
			{ Height(1), 20 },
			{ Height(56), 20 }, // (1 + 112) / 2
			{ Height(84), 20 }, // (56 + 112) / 2
			{ Height(98), 20 } // (84 + 112) / 2
		});
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenChainsDivergeAndRemoteHasHigherScoreAndLastFinalizedBlockIsNotNemesis_MultiBatch) {
		AssertRemoteIsNotSynced(Height(7), 100, {
			{ Height(7), 20 },
			{ Height(62), 20 }, // (7 + 118) / 2
			{ Height(90), 20 }, // (62 + 118) / 2
			{ Height(104), 20 } // (90 + 118) / 2
		});
	}

	// endregion

	// region hash - sync during compare [block]

	HASH_TEST(RemoteIsNotPunishedWhenLocalNodeSyncsToBlockEqualToRemoteDuringChainCompare) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 10));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 10));
		local.syncHashes(remote, 0, 0);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(ChainScore(11)); // as part of sync, score should equal remote

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert: remote was not punished
		EXPECT_EQ(ChainComparisonCode::Local_Score_Updated, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteIsNotPunishedWhenLocalNodeSyncsToBlockBetterThanRemoteDuringChainCompare) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 10));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 10));
		local.syncHashes(remote, -1, 1);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(ChainScore(12)); // as part of sync, score should be better than remote

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert: since remote chain score is worse, this will lead to rejection during block processing
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(adjustment + 9), result.CommonBlockHeight);
		EXPECT_EQ(1u, result.ForkDepth);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalNodeSyncsToBlockWorseThanRemoteDuringChainCompare) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 10));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 10));
		local.syncHashes(remote, -1, 1);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(ChainScore(9)); // as part of sync, score should be worse than remote

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(adjustment + 9), result.CommonBlockHeight);
		EXPECT_EQ(1u, result.ForkDepth);
	}

	// endregion

	// region hash - sync during compare [chain]

	HASH_TEST(RemoteIsNotPunishedWhenLocalNodeSyncsToChainEqualToRemoteDuringChainCompare) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 13 local hashes
		// - 3. return X + 13 remote hashes
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 8));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 13));
		local.syncHashes(remote, 0, 0);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(ChainScore(11)); // as part of sync, score should equal remote

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert: remote was not punished
		EXPECT_EQ(ChainComparisonCode::Local_Score_Updated, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteIsNotPunishedWhenLocalNodeSyncsToChainBetterThanRemoteDuringChainCompare) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 14 local hashes
		// - 3. return X + 13 remote hashes
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 8));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 13));
		local.syncHashes(remote, 0, 1);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(ChainScore(12)); // as part of sync, score should be better than remote

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert: remote was not punished
		EXPECT_EQ(ChainComparisonCode::Local_Score_Updated, result.Code);
		AssertDefaultChainStatistics(result);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalNodeSyncsToChainWorseThanRemoteDuringChainCompare) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 12 local hashes
		// - 3. return X + 13 remote hashes
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 8));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 13));
		local.syncHashes(remote, -1, 0);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(ChainScore(9)); // as part of sync, score should be worse than remote

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(adjustment + 12), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChainButReturnsMoreHashesThanExpected) {
		// Arrange: simulate the scenario where the local height increases in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 12 local hashes
		// - 3. return X + 13 remote hashes so that the remote still has a better score
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 8));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 13));
		local.syncHashes(remote, -1, 0);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(adjustment + 12), result.CommonBlockHeight);
		EXPECT_EQ(0u, result.ForkDepth);
	}

	HASH_TEST(RemoteIsNotSyncedWhenLocalIsSmallerThanRemoteChainAndContainsAllHashesInRemoteChainButReturnsFewerHashesThanExpected) {
		// Arrange: simulate the scenario where the local height decreases in-between the height and hashes requests
		// - 1. report local height as X + 8
		// - 2. return X + 6 local hashes
		// - 3. return X + 13 remote hashes so that the remote still has a better score
		auto adjustment = TTraits::Prepare_Adjustment;

		CompareHashesMockChainApi local(ChainScore(10), Height(adjustment + 8));
		CompareHashesMockChainApi remote(ChainScore(11), Height(adjustment + 13));
		local.syncHashes(remote, -7, 0);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get();

		// Assert: fork depth is based on original (not current) local height, so this will likely lead to unlinked error downstream
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(adjustment + 6), result.CommonBlockHeight);
		EXPECT_EQ(2u, result.ForkDepth);
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

	TEST(TEST_CLASS, LocalChainStatisticsExceptionIsPropagatedDuringLocalScoreCheck) {
		// Arrange: simulate the scenario where the local node syncs in-between the height and hashes requests
		CompareHashesMockChainApi local(ChainScore(10), Height(10));
		CompareHashesMockChainApi remote(ChainScore(11), Height(10));
		local.syncHashes(remote, 0, 0);
		local.pushChainScore(ChainScore(10));
		local.pushChainScore(CompareHashesMockChainApi::ChainScoreExceptionTrigger());

		// Act:
		EXPECT_THROW(CompareChains(local, remote, CreateCompareChainsOptions(20, 1)).get(), catapult_runtime_error);
	}

	// endregion

	// region hash (multiple batches)

	namespace {
		void AssertRemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchButRemoteHasMultipleBatches(uint64_t adjustment) {
			// Arrange: Local { ..., A, B, C || }, Remote { ..., A, B, C || D }; where || indicates end of batch
			static constexpr auto Batch_Size = static_cast<uint32_t>(20);
			static constexpr auto Finalized_Height = Height(3);

			CompareHashesMockChainApi local(ChainScore(10), Finalized_Height + Height(Batch_Size - 1 - adjustment));
			CompareHashesMockChainApi remote(ChainScore(11), Finalized_Height + Height(Batch_Size));
			local.syncHashes(remote, -1, 0);

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(Batch_Size, Finalized_Height.unwrap())).get();

			// Assert:
			EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
			EXPECT_EQ(Height(22), result.CommonBlockHeight);
			EXPECT_EQ(0u, result.ForkDepth);

			// - check requests (an additional request is needed to prove remote has more hashes than local)
			auto expectedHashesFromRequests = std::vector<std::pair<Height, uint32_t>>{
				{ Finalized_Height, Batch_Size },
				{ Finalized_Height + Height(Batch_Size - 2), Batch_Size }
			};
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

	TEST(TEST_CLASS, RemoteIsNotSyncedWhenLocalContainsExactlyFirstBatchButRemoteHasMultipleBatchesButThenSyncs) {
		// Arrange: Local { ..., A, B, C || }, Remote { ..., A, B, C || D }; where || indicates end of batch
		static constexpr auto Batch_Size = static_cast<uint32_t>(20);
		static constexpr auto Finalized_Height = Height(3);

		auto baseHashes = test::GenerateRandomHashes(19);
		auto remoteHashes = test::GenerateRandomHashes(1);
		auto commonHashes = test::ConcatHashes(baseHashes, remoteHashes);

		MockChainApi local(ChainScore(10), Finalized_Height + Height(Batch_Size - 1));
		local.setHashes(Finalized_Height, commonHashes);
		local.setHashes(Finalized_Height + Height(Batch_Size - 2), remoteHashes);

		MockChainApi remote(ChainScore(11), Finalized_Height + Height(Batch_Size));
		remote.setHashes(Finalized_Height, commonHashes);
		remote.setHashes(Finalized_Height + Height(Batch_Size - 2), remoteHashes); // lie and return only a single hash

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(Batch_Size, Finalized_Height.unwrap())).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Lied_About_Chain_Score, result.Code);
		AssertDefaultChainStatistics(result);
	}

	namespace {
		void AssertRemoteIsNotSyncedDeepFork(
				uint32_t commonBlockHeight,
				const std::vector<std::pair<Height, uint32_t>>& expectedHashesFromRequests) {
			// Arrange: Local { ..., A, B, C, D, ... }, Remote { ..., A, B, C, E, ... }
			static constexpr auto Finalized_Height = Height(7);

			CompareHashesMockChainApi local(ChainScore(10), Finalized_Height + Height(111));
			CompareHashesMockChainApi remote(ChainScore(11), Finalized_Height + Height(111));
			remote.syncHashes(local, static_cast<int32_t>(commonBlockHeight) - 118, 118 - commonBlockHeight);

			// Act:
			auto result = CompareChains(local, remote, CreateCompareChainsOptions(20, Finalized_Height.unwrap())).get();

			// Assert:
			EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
			EXPECT_EQ(Height(commonBlockHeight), result.CommonBlockHeight);
			EXPECT_EQ(118u - commonBlockHeight, result.ForkDepth);

			// - check requests
			EXPECT_EQ(expectedHashesFromRequests, local.hashesFromRequests());
			EXPECT_EQ(expectedHashesFromRequests, remote.hashesFromRequests());
		}
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedDeepForkScenario1_MultiBatch) {
		AssertRemoteIsNotSyncedDeepFork(90, {
			{ Height(7), 20 },
			{ Height(62), 20 }, // (7 + 118) / 2
			{ Height(90), 20 } // (62 + 118) / 2
		});
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedDeepForkScenario2_MultiBatch) {
		AssertRemoteIsNotSyncedDeepFork(89, {
			{ Height(7), 20 },
			{ Height(62), 20 }, // (7 + 118) / 2
			{ Height(90), 20 }, // (62 + 118) / 2
			{ Height(76), 20 } // (62 + 90) / 2
		});
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedDeepForkScenario3_MultiBatch) {
		AssertRemoteIsNotSyncedDeepFork(34, {
			{ Height(7), 20 },
			{ Height(62), 20 }, // (7 + 118) / 2
			{ Height(34), 20 } // (7 + 62) / 2
		});
	}

	TEST(TEST_CLASS, RemoteIsNotSyncedDeepForkScenario4_MultiBatch) {
		AssertRemoteIsNotSyncedDeepFork(54, {
			{ Height(7), 20 },
			{ Height(62), 20 }, // (7 + 118) / 2
			{ Height(34), 20 }, // (7 + 62) / 2
			{ Height(48), 20 } // (34 + 62) / 2
		});
	}

	// endregion

	// region regression tests

	TEST(TEST_CLASS, RemoteHasSmallerBatchSizeThanLocal) {
		// Arrange: Local { ..., A, B }, Remote { ..., C, D, E, F }
		// - batch sizes: { remote = 100, local = 1210 }
		CompareHashesMockChainApi local(ChainScore(10), Height(4261));
		CompareHashesMockChainApi remote(ChainScore(11), Height(4263));
		local.syncHashes(remote, -4, 2);
		remote.setMaxHashesBatchSize(100);

		// Act:
		auto result = CompareChains(local, remote, CreateCompareChainsOptions(1210, 1)).get();

		// Assert:
		EXPECT_EQ(ChainComparisonCode::Remote_Is_Not_Synced, result.Code);
		EXPECT_EQ(Height(4259), result.CommonBlockHeight);
		EXPECT_EQ(2u, result.ForkDepth);

		// - check requests
		auto expectedHashesFromRequests = std::vector<std::pair<Height, uint32_t>>{
			{ Height(1), 1210 },
			{ Height(2131), 1210 },
			{ Height(3196), 1210 },
			{ Height(3728), 1210 },
			{ Height(3994), 1210 },
			{ Height(4127), 1210 },
			{ Height(4194), 1210 }
		};
		EXPECT_EQ(expectedHashesFromRequests, local.hashesFromRequests());
		EXPECT_EQ(expectedHashesFromRequests, remote.hashesFromRequests());
	}

	// endregion
}}
