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

#include "catapult/chain/ChainSynchronizer.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/ChainScore.h"
#include "catapult/model/EntityRange.h"
#include "tests/catapult/chain/test/MockChainApi.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

using namespace catapult::model;
using catapult::mocks::MockPacketIo;
using catapult::mocks::MockChainApi;

namespace catapult { namespace chain {

#define TEST_CLASS ChainSynchronizerTests

	namespace {
		constexpr auto Default_Height = Height(20);
		constexpr auto Last_Finalized_Height = Height(11);

		// region test context

		struct TestContext {
		public:
			TestContext(const ChainScore& localScore, const ChainScore& remoteScore)
					: TestContext(localScore, remoteScore, {}, {}, test::GenerateBlockWithTransactions(0, Default_Height))
			{}

			TestContext(const ChainScore& localScore, const ChainScore& remoteScore, std::unique_ptr<Block>&& pRemoteLastBlock)
					: TestContext(localScore, remoteScore, {}, {}, std::move(pRemoteLastBlock))
			{}

			TestContext(
					const ChainScore& localScore,
					const ChainScore& remoteScore,
					const HashRange& localHashes,
					const HashRange& remoteHashes,
					std::unique_ptr<Block>&& pRemoteLastBlock)
					: LocalScore(localScore)
					, LocalHashes(HashRange::CopyRange(localHashes))
					, pIo(std::make_shared<MockPacketIo>())
					, pChainApi(std::make_shared<MockChainApi>(remoteScore, std::move(pRemoteLastBlock)))
					, BlockRangeConsumerCalls(0)
					, Config(CreateConfiguration()) {
				pChainApi->setHashes(Last_Finalized_Height, remoteHashes);
			}

		public:
			void assertNoCalls() const {
				EXPECT_EQ(0u, BlockRangeConsumerCalls);
			}

		private:
			static ChainSynchronizerConfiguration CreateConfiguration() {
				auto config = ChainSynchronizerConfiguration();
				config.MaxHashesPerSyncAttempt = 4 * 100;
				config.MaxBlocksPerSyncAttempt = 4 * 100;
				config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512).bytes32();
				return config;
			}

		public:
			ChainScore LocalScore;
			HashRange LocalHashes;
			std::shared_ptr<MockPacketIo> pIo;
			std::shared_ptr<MockChainApi> pChainApi;
			size_t BlockRangeConsumerCalls;
			std::vector<model::NodeIdentity> BlockRangeSourceIdentities;
			ChainSynchronizerConfiguration Config;
			disruptor::ProcessingCompleteFunc ProcessingComplete;
		};

		// endregion

		// region test utils

		enum class ConsumerMode { Normal, Full };

		RemoteNodeSynchronizer<api::RemoteChainApi> CreateSynchronizer(TestContext& context, ConsumerMode mode = ConsumerMode::Normal) {
			auto pVerifiableBlock = test::GenerateBlockWithTransactions(0, Default_Height);
			auto pLocal = std::make_shared<MockChainApi>(context.LocalScore, std::move(pVerifiableBlock));
			pLocal->setHashes(Last_Finalized_Height, context.LocalHashes);

			auto finalizedHeightSupplier = []() { return Last_Finalized_Height; };

			auto blockRangeConsumer = [mode, &context](const auto& range, const auto& processingComplete) {
				++context.BlockRangeConsumerCalls;
				context.BlockRangeSourceIdentities.push_back(range.SourceIdentity);
				context.ProcessingComplete = processingComplete;
				return ConsumerMode::Normal == mode ? context.BlockRangeConsumerCalls : 0;
			};

			return CreateChainSynchronizer(pLocal, context.Config, finalizedHeightSupplier, blockRangeConsumer);
		}

		disruptor::ConsumerCompletionResult CreateContinueResult() {
			disruptor::ConsumerCompletionResult result;
			result.CompletionStatus = disruptor::CompletionStatus::Normal;
			return result;
		}

		disruptor::ConsumerCompletionResult CreateAbortResult() {
			disruptor::ConsumerCompletionResult result;
			result.CompletionStatus = disruptor::CompletionStatus::Aborted;
			return result;
		}

		void AssertSync(const TestContext& context, size_t numBlockConsumerCalls) {
			EXPECT_EQ(numBlockConsumerCalls, context.BlockRangeConsumerCalls);
			EXPECT_EQ(numBlockConsumerCalls, context.BlockRangeSourceIdentities.size());

			auto i = 0u;
			for (const auto& sourceIdentity : context.BlockRangeSourceIdentities) {
				EXPECT_NE(Key(), sourceIdentity.PublicKey) << "key at " << i;
				EXPECT_EQ(context.pChainApi->remoteIdentity().PublicKey, sourceIdentity.PublicKey) << "key at " << i;

				EXPECT_EQ("fake-host-from-mock-chain-api", sourceIdentity.Host) << "host at " << i;
				EXPECT_EQ(context.pChainApi->remoteIdentity().Host, sourceIdentity.Host) << "host at " << i;
				++i;
			}
		}

		// endregion
	}

	// region chain synchronization - compare chains (neutral)

	namespace {
		void AssertNeutralInteraction(ChainScore localScore, ChainScore remoteScore) {
			// Arrange:
			TestContext context(localScore, remoteScore);
			auto synchronizer = CreateSynchronizer(context);

			// Act:
			auto code = synchronizer(*context.pChainApi).get();

			// Assert:
			EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, code);
			EXPECT_EQ(0u, context.BlockRangeConsumerCalls);
		}
	}

	TEST(TEST_CLASS, NeutralInteractionWhenCompareChainsReturnsEqualScoreResult) {
		AssertNeutralInteraction(ChainScore(10), ChainScore(10));
	}

	TEST(TEST_CLASS, NeutralInteractionWhenCompareChainsReturnsLowerScoreResult) {
		AssertNeutralInteraction(ChainScore(11), ChainScore(10));
	}

	// endregion

	// region chain synchronization - compare chains (failed)

	namespace {
		void AssertFailedInteraction(ChainScore localScore, ChainScore remoteScore, std::unique_ptr<Block>&& pRemoteLastBlock) {
			// Arrange:
			TestContext context(localScore, remoteScore, std::move(pRemoteLastBlock));
			auto synchronizer = CreateSynchronizer(context);

			// Act:
			auto code = synchronizer(*context.pChainApi).get();

			// Assert:
			EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, code);
			context.assertNoCalls();
		}
	}

	TEST(TEST_CLASS, FailedInteractionWhenCompareChainsReturnsNonRecoverableOutOfSyncResult) {
		// Assert: trigger failure by letting the remote return a block too far behind
		//         (using wraparound to force this condition)
		AssertFailedInteraction(ChainScore(10), ChainScore(11), test::GenerateBlockWithTransactions(0, Default_Height - Height(361)));
	}

	TEST(TEST_CLASS, FailedInteractionWhenCompareChainsReturnsEvilResult) {
		// Assert: trigger failure by letting the remote return a non-verifiable last block
		AssertFailedInteraction(ChainScore(10), ChainScore(11), test::GenerateBlockWithTransactions(0, Default_Height));
	}

	TEST(TEST_CLASS, FailedInteractionWhenCompareChainsReturnsException) {
		// Arrange:
		TestContext context(ChainScore(10), ChainScore(11));
		context.pChainApi->setError(MockChainApi::EntryPoint::Chain_Statistics);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, code);
		context.assertNoCalls();
	}

	// endregion

	// region chain synchronization

	namespace {
		TestContext CreateTestContextWithHashes(size_t numLocalHashes, size_t numRemoteHashes, size_t forkDepth = 0) {
			auto remoteHashes = test::GenerateRandomHashes(numRemoteHashes);
			auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, numLocalHashes);
			localHashes = test::ConcatHashes(localHashes, test::GenerateRandomHashes(forkDepth));

			TestContext context(
					ChainScore(10),
					ChainScore(11),
					localHashes,
					remoteHashes,
					test::GenerateBlockWithTransactions(0, Default_Height));
			context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2 });
			context.Config.MaxHashesPerSyncAttempt = 10;
			context.Config.MaxBlocksPerSyncAttempt = 5;
			context.Config.MaxChainBytesPerSyncAttempt = 23;
			return context;
		}

		void AssertDefaultSinglePullRequest(const mocks::MockChainApi& chainApi) {
			// Assert:
			ASSERT_EQ(1u, chainApi.blocksFromRequests().size());
			const auto& params = chainApi.blocksFromRequests()[0];
			EXPECT_EQ(Default_Height, params.first);
			EXPECT_EQ(5u, params.second.NumBlocks); // maxBlocksPerSyncAttempt
			EXPECT_EQ(23u, params.second.NumBytes);
		}

		void AssertDefaultMultiplePullRequest(const mocks::MockChainApi& chainApi, const std::vector<Height>& expectedRequestHeights) {
			// Assert:
			ASSERT_EQ(expectedRequestHeights.size(), chainApi.blocksFromRequests().size());

			auto i = 0u;
			for (const auto& params : chainApi.blocksFromRequests()) {
				EXPECT_EQ(expectedRequestHeights[i], params.first) << "height of request " << i;
				EXPECT_EQ(5u, params.second.NumBlocks) << "NumBlocks of request " << i; // maxBlocksPerSyncAttempt
				EXPECT_EQ(23u, params.second.NumBytes) << "NumBytes of request " << i;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, SuccessfulInteractionWhenRemoteChainPartWasSuccessfullyPulled) {
		// Arrange:
		auto context = CreateTestContextWithHashes(9, 10);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
		AssertSync(context, 1);
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, NeutralInteractionWhenRemoteChainPartWasSuccessfullyPulledAndConsumerIsFull) {
		// Arrange: simulate a full consumer
		auto context = CreateTestContextWithHashes(9, 10);
		auto synchronizer = CreateSynchronizer(context, ConsumerMode::Full);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert: neutral because blocks could not be processed, so chain is unchanged
		EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, code);
		AssertSync(context, 1);
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, SuccessfulInteractionWithMultiplePulls) {
		// Arrange:
		// - common block has height 10 + 4 (fork depth 6)
		// - pulls 2 blocks at time: 3 attempts needed to pull 6 blocks
		constexpr auto Common_Block_Height = Height(14);

		auto context = CreateTestContextWithHashes(4, 10, 6);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
		AssertSync(context, 1);
		AssertDefaultMultiplePullRequest(*context.pChainApi, {
			Common_Block_Height + Height(1),
			Common_Block_Height + Height(3),
			Common_Block_Height + Height(5)
		});
	}

	TEST(TEST_CLASS, NeutralInteractionWhenRemoteDoesNotHaveBlocksAtRequestedHeight) {
		// Arrange:
		auto context = CreateTestContextWithHashes(9, 10);
		context.pChainApi->setNumBlocksPerBlocksFromRequest({ 0 });
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, code);
		context.assertNoCalls();
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, SuccessfulInteractionWithMultiplePullsWhenRemoteRunsOutOfBlocks) {
		// Arrange:
		// - common block has height 10 + 2 (fork depth 8)
		// - pulls 2 blocks at time: 4 attempts needed to pull 8 blocks but blocks are only returned twice
		//   (third attempt returns no blocks, so subsequent attempts are bypassed)
		constexpr auto Common_Block_Height = Height(12);

		auto context = CreateTestContextWithHashes(2, 10, 8);
		context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2, 2, 0 });
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
		AssertSync(context, 1);
		AssertDefaultMultiplePullRequest(*context.pChainApi, {
			Common_Block_Height + Height(1),
			Common_Block_Height + Height(3),
			Common_Block_Height + Height(5)
		});
	}

	TEST(TEST_CLASS, FailedInteractionWhenBlocksFromReturnsException) {
		// Arrange:
		auto context = CreateTestContextWithHashes(9, 10);
		context.pChainApi->setError(MockChainApi::EntryPoint::Blocks_From);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto code = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, code);
		context.assertNoCalls();
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, ReturnsNotReadyFutureWhenPullingBlocks) {
		// Arrange:
		auto context = CreateTestContextWithHashes(9, 10);
		context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto future = synchronizer(*context.pChainApi);

		// Assert:
		EXPECT_FALSE(future.is_ready());

		// Cleanup: wait for future to complete
		future.get();
	}

	// endregion

	// region unprocessed elements

	namespace {
		enum class ChainScoreRelation {
			RemoteBetter,
			Equal
		};

		auto CreateTestContextForUnprocessedElementTests(ChainScoreRelation relation = ChainScoreRelation::RemoteBetter) {
			auto remoteHashes = test::GenerateRandomHashes(10);
			auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 9);
			auto context = TestContext(
					ChainScore(10),
					ChainScoreRelation::RemoteBetter == relation ? ChainScore(11) : ChainScore(10),
					localHashes,
					remoteHashes,
					test::GenerateBlockWithTransactions(0, Default_Height));
			context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2 });
			return context;
		}

		void AssertRequestHeight(const TestContext& context, size_t index, Height expectedHeight) {
			EXPECT_EQ(expectedHeight, context.pChainApi->blocksFromRequests()[index].first)
					<< "height for request " << (index + 1);
		}

		void AssertRequestHeights(const TestContext& context, const std::vector<Height>& expectedHeights) {
			ASSERT_EQ(expectedHeights.size(), context.pChainApi->blocksFromRequests().size());

			auto i = 0u;
			for (auto expectedHeight : expectedHeights)
				AssertRequestHeight(context, i++, expectedHeight);
		}
	}

	TEST(TEST_CLASS, SuccessfulInteractionWhenContainerIsNeitherFullNorDirty) {
		// Arrange: by default the container max size is large enough to pass many sync rounds without being full
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;

		// Act:
		for (auto i = 0u; i < 10; i++)
			interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		AssertSync(context, 10);

		auto i = 0u;
		ASSERT_EQ(10u, context.pChainApi->blocksFromRequests().size());
		for (auto code : interactionResultCodes) {
			EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
			AssertRequestHeight(context, i, Default_Height + Height(2 * i));
			++i;
		}
	}

	TEST(TEST_CLASS, NeutralInteractionWhenContainerIsFull) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		context.Config.MaxChainBytesPerSyncAttempt = sizeof(BlockHeader) / 3 - 1;
		auto synchronizer = CreateSynchronizer(context);

		// Act: second call is short circuited since the container is full
		auto code1 = synchronizer(*context.pChainApi).get();
		auto code2 = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(code1, ionet::NodeInteractionResultCode::Success);
		EXPECT_EQ(code2, ionet::NodeInteractionResultCode::Neutral);
		AssertSync(context, 1);

		AssertRequestHeights(context, { Default_Height });
	}

	TEST(TEST_CLASS, SuccessfulInteractionWhenPreviouslyFullContainerIsNoLongerFull) {
		// Arrange: the container's max size is set to 3 * MaxChainBytesPerSyncAttempt = 3 * sizeof(BlockHeader)
		//          that means the container is full after 2 syncs (2 blocks per sync)
		auto context = CreateTestContextForUnprocessedElementTests();
		context.Config.MaxChainBytesPerSyncAttempt = sizeof(BlockHeader);
		auto synchronizer = CreateSynchronizer(context);
		std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;

		// - sucessful since container is not full
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// - neutral since container is full
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for first element has finished
		context.ProcessingComplete(1, CreateContinueResult());

		// Act: since an unprocessed element was removed, the container is no longer full
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Neutral,
			ionet::NodeInteractionResultCode::Success
		};

		AssertSync(context, 3);
		EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);

		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2), Default_Height + Height(4) });
	}

	TEST(TEST_CLASS, NeutralInteractionWhenContainerIsDirty) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for first element has finished unsuccessfully making the container dirty
		context.ProcessingComplete(1, CreateAbortResult());

		// Act: neutral because container is dirty
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Neutral,
			ionet::NodeInteractionResultCode::Neutral
		};

		AssertSync(context, 2);
		EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);

		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2) });
	}

	TEST(TEST_CLASS, DirtyContainerIsCleanAfterAllElementsHaveBeenRemoved) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for first element has finished unsuccessfully making the container dirty
		context.ProcessingComplete(1, CreateAbortResult());

		// - neutral because container is dirty
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for second element has finished. The container is empty and thus clean again
		context.ProcessingComplete(2, CreateAbortResult());

		// Act: successful because container is clean
		interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Neutral,
			ionet::NodeInteractionResultCode::Success
		};

		AssertSync(context, 3);
		EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);

		// - last request should be Default_Height because container is empty again
		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2), Default_Height });
	}

	TEST(TEST_CLASS, PendingBlocksAreMarkedAsDirtyWhenRootBlocksAreDirty) {
		// Arrange: non-deterministic because of dependency on delay
		test::RunNonDeterministicTest("closes socket test", [](auto i) {
			// Arrange:
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);
			std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;

			// Act: start an (immediate) request
			interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

			// - start a delayed request
			context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10 * i));
			auto syncFuture = synchronizer(*context.pChainApi);

			// - signal processing for first element has finished unsuccessfully making the container dirty
			context.ProcessingComplete(1, CreateAbortResult());

			// - wait for the delayed request (it must still be pending in order for this test to give a deterministic result)
			if (syncFuture.is_ready())
				return false;

			interactionResultCodes.push_back(syncFuture.get());

			// Assert:
			std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
				ionet::NodeInteractionResultCode::Success,
				ionet::NodeInteractionResultCode::Neutral
			};

			// - only one range was forwarded to the consumer because the second was rejected
			AssertSync(context, 1);
			EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);

			AssertRequestHeights(context, { Default_Height, Default_Height + Height(2) });
			return true;
		});
	}

	TEST(TEST_CLASS, MultiplePendingSyncsAreNotAllowedAtOnce) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;

		// Act: start two delayed requests
		context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
		auto syncFuture1 = synchronizer(*context.pChainApi);
		auto syncFuture2 = synchronizer(*context.pChainApi);

		// - wait for the delayed requests
		interactionResultCodes.push_back(syncFuture1.get());
		interactionResultCodes.push_back(syncFuture2.get());

		// Assert: only one outstanding sync is allowed at a time
		std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
			ionet::NodeInteractionResultCode::Success,
			ionet::NodeInteractionResultCode::Neutral
		};

		// - only one range was forwarded to the consumer because the second was bypassed
		AssertSync(context, 1);
		EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);

		// - only one request was made because the second was bypassed
		AssertRequestHeights(context, { Default_Height });
	}

	TEST(TEST_CLASS, ThrowsWhenCompletionHandlerIsCalledWithUnknownElementId) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		auto future = synchronizer(*context.pChainApi);

		// Act + Assert: signal processing has finished with unknown element id
		EXPECT_THROW(context.ProcessingComplete(123, CreateContinueResult()), catapult_invalid_argument);
	}

	// endregion

	// region recoverability

	namespace {
		void AssertSyncRecoverablityAfterChainApiException(
				MockChainApi::EntryPoint errorPoint,
				const std::vector<Height>& expectedRequestHeights) {
			// Arrange:
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);
			std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;

			// Act: set an exception and sync
			context.pChainApi->setError(errorPoint);
			interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

			// - clear the exception and sync
			context.pChainApi->setError(MockChainApi::EntryPoint::None);
			interactionResultCodes.push_back(synchronizer(*context.pChainApi).get());

			// Assert: the first sync failed but the second succeeded
			std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
				ionet::NodeInteractionResultCode::Failure,
				ionet::NodeInteractionResultCode::Success
			};

			AssertSync(context, 1);
			EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);
			AssertRequestHeights(context, expectedRequestHeights);
		}
	}

	TEST(TEST_CLASS, CanRecoverAfterFailedCompareChainsOperation) {
		// Assert: only one blocks-from request was made because the chain comparison failed
		AssertSyncRecoverablityAfterChainApiException(MockChainApi::EntryPoint::Chain_Statistics, { Default_Height });
	}

	TEST(TEST_CLASS, CanRecoverAfterFailedBlocksFromOperation) {
		// Assert: two blocks-from requests were made beacuse the the chain comparison succeeded
		AssertSyncRecoverablityAfterChainApiException(MockChainApi::EntryPoint::Blocks_From, { Default_Height, Default_Height });
	}

	// endregion

	// region clean shutdown

	TEST(TEST_CLASS, SynchronizerFutureCanCompleteAfterSynchronizerIsDestroyed) {
		// Arrange:
		thread::future<ionet::NodeInteractionResultCode> future;
		auto context = CreateTestContextWithHashes(9, 10);
		{
			context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
			auto synchronizer = CreateSynchronizer(context);

			// Act: start synchronizing
			future = synchronizer(*context.pChainApi);

			// - destroy the synchronizer before completion
			CATAPULT_LOG(debug) << "destroying synchronizer";
		}

		// Sanity: future has not yet completed
		EXPECT_FALSE(future.is_ready());

		// Assert: future can complete
		future.get();
	}

	TEST(TEST_CLASS, BlockRangeConsumerCanCompleteAfterSynchronizerIsDestroyed) {
		// Arrange:
		disruptor::ProcessingCompleteFunc processingComplete;
		{
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);

			// - add an element to unprocessed elements and capture the completion function
			synchronizer(*context.pChainApi).get();
			processingComplete = context.ProcessingComplete;

			// Act: destroy the synchronizer before completion
			CATAPULT_LOG(debug) << "destroying synchronizer";
		}

		// Assert: the completion function can still be called even though the synchronizer has been destroyed
		processingComplete(1, CreateAbortResult());
	}

	// endregion
}}
