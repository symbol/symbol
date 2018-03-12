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
		constexpr Height Default_Height(20);

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

		ChainSynchronizerConfiguration CreateConfiguration() {
			auto config = ChainSynchronizerConfiguration();
			config.MaxBlocksPerSyncAttempt = 4 * 100;
			config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512).bytes32();
			config.MaxRollbackBlocks = 360;
			return config;
		}

		struct TestContext {
			TestContext(const ChainScore& localScore, const ChainScore& remoteScore)
					: TestContext(localScore, remoteScore, {}, {}, test::GenerateVerifiableBlockAtHeight(Default_Height))
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
					, pChainApi(std::make_shared<MockChainApi>(remoteScore, std::move(pRemoteLastBlock), remoteHashes))
					, BlockRangeConsumerCalls(0)
					, Config(CreateConfiguration())
			{}

			void assertNoCalls() const {
				EXPECT_EQ(0u, BlockRangeConsumerCalls);
			}

			ChainScore LocalScore;
			HashRange LocalHashes;
			std::shared_ptr<MockPacketIo> pIo;
			std::shared_ptr<MockChainApi> pChainApi;
			size_t BlockRangeConsumerCalls;
			ChainSynchronizerConfiguration Config;
			disruptor::ProcessingCompleteFunc ProcessingComplete;
		};

		enum class ConsumerMode { Normal, Full };

		RemoteNodeSynchronizer<api::RemoteChainApi> CreateSynchronizer(TestContext& context, ConsumerMode mode = ConsumerMode::Normal) {
			auto pVerifiableBlock = test::GenerateVerifiableBlockAtHeight(Default_Height);
			auto pLocal = std::make_shared<MockChainApi>(context.LocalScore, std::move(pVerifiableBlock), context.LocalHashes);

			auto& blockConsumerCalls = context.BlockRangeConsumerCalls;
			auto blockRangeConsumer = [mode, &blockConsumerCalls, &context](const auto&, const auto& processingComplete) {
				++blockConsumerCalls;
				context.ProcessingComplete = processingComplete;
				return ConsumerMode::Normal == mode ? blockConsumerCalls : 0;
			};

			return CreateChainSynchronizer(pLocal, context.Config, blockRangeConsumer);
		}

		void AssertSync(const TestContext& context, size_t numBlockConsumerCalls) {
			EXPECT_EQ(numBlockConsumerCalls, context.BlockRangeConsumerCalls);
		}
	}

	// region chain synchronization

	namespace {
		void AssertNeutralInteraction(ChainScore localScore, ChainScore remoteScore) {
			// Arrange:
			TestContext context(localScore, remoteScore);
			auto synchronizer = CreateSynchronizer(context);

			// Act:
			auto result = synchronizer(*context.pChainApi).get();

			// Assert:
			EXPECT_EQ(NodeInteractionResult::Neutral, result);
			EXPECT_EQ(0u, context.BlockRangeConsumerCalls);
		}
	}

	TEST(TEST_CLASS, NeutralInteractionIfCompareChainsReturnsEqualScoreResult) {
		// Assert:
		AssertNeutralInteraction(ChainScore(10), ChainScore(10));
	}

	TEST(TEST_CLASS, NeutralInteractionIfCompareChainsReturnsLowerScoreResult) {
		// Assert:
		AssertNeutralInteraction(ChainScore(11), ChainScore(10));
	}

	TEST(TEST_CLASS, FailedInteractionIfCompareChainsReturnsFailureResult) {
		// Arrange: trigger failure by letting the remote return a non-verifiable last block
		TestContext context(ChainScore(10), ChainScore(11), test::GenerateNonVerifiableBlockAtHeight(Default_Height));
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		context.assertNoCalls();
	}

	TEST(TEST_CLASS, FailedInteractionIfCompareChainsReturnsException) {
		// Arrange:
		TestContext context(ChainScore(10), ChainScore(11));
		context.pChainApi->setError(MockChainApi::EntryPoint::Chain_Info);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		context.assertNoCalls();
	}

	namespace {
		auto CreateDefaultTestContext(size_t numLocalHashes, size_t numRemoteHashes, size_t forkDepth = 0) {
			auto remoteHashes = test::GenerateRandomHashes(numRemoteHashes);
			auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, numLocalHashes);
			localHashes = test::ConcatHashes(localHashes, test::GenerateRandomHashes(forkDepth));

			TestContext context(
					ChainScore(10),
					ChainScore(11),
					localHashes,
					remoteHashes,
					test::GenerateVerifiableBlockAtHeight(Default_Height));
			context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2 });
			context.Config.MaxRollbackBlocks = 9;
			context.Config.MaxChainBytesPerSyncAttempt = 23;
			return context;
		}

		void AssertDefaultSinglePullRequest(const mocks::MockChainApi& chainApi) {
			// Assert:
			ASSERT_EQ(1u, chainApi.blocksFromRequests().size());
			const auto& params = chainApi.blocksFromRequests()[0];
			EXPECT_EQ(Default_Height, params.first); // localHeight - maxRollback + firstDifferenceIndex
			EXPECT_EQ(9, params.second.NumBlocks); // maxRollback
			EXPECT_EQ(23, params.second.NumBytes);
		}

		void AssertDefaultMultiplePullRequest(const mocks::MockChainApi& chainApi, const std::vector<Height>& expectedRequestHeights) {
			// Assert:
			ASSERT_EQ(expectedRequestHeights.size(), chainApi.blocksFromRequests().size());

			auto i = 0u;
			for (const auto& params : chainApi.blocksFromRequests()) {
				EXPECT_EQ(expectedRequestHeights[i], params.first) << "height of request " << i;
				EXPECT_EQ(9, params.second.NumBlocks) << "NumBlocks of request " << i; // maxRollback
				EXPECT_EQ(23, params.second.NumBytes) << "NumBytes of request " << i;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, SuccessfulInteractionIfRemoteChainPartWasSuccessfullyPulled) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Success, result);
		AssertSync(context, 1);
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, NeutralInteractionIfRemoteChainPartWasSuccessfullyPulledAndConsumerIsFull) {
		// Arrange: simulate a full consumer
		auto context = CreateDefaultTestContext(9, 10);
		auto synchronizer = CreateSynchronizer(context, ConsumerMode::Full);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert: neutral because blocks could not be processed, so chain is unchanged
		EXPECT_EQ(NodeInteractionResult::Neutral, result);
		AssertSync(context, 1);
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, SuccessfulInteractionWithMultiplePulls) {
		// Arrange:
		// - last block has height 20, rewrite limit is 9
		// - common block has height 14 = 20 - 9 + 4 - 1 (fork depth 6)
		// - pulls 2 blocks at time: 3 attempts needed to pull 6 blocks
		auto context = CreateDefaultTestContext(4, 10, 6);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Success, result);
		AssertSync(context, 1);
		AssertDefaultMultiplePullRequest(*context.pChainApi, { Height(15), Height(17), Height(19) });
	}

	TEST(TEST_CLASS, NeutralInteractionIfRemoteDoesNotHaveBlocksAtRequestedHeight) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		context.pChainApi->setNumBlocksPerBlocksFromRequest({ 0 });
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Neutral, result);
		context.assertNoCalls();
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, SuccessfulInteractionWithMultiplePullsWhenRemoteRunsOutOfBlocks) {
		// Arrange:
		// - last block has height 20, rewrite limit is 9
		// - common block has height 12 = 20 - 9 + 2 - 1 (fork depth 8)
		// - pulls 2 blocks at time: 4 attempts needed to pull 8 blocks but blocks are only returned twice
		//   (third attempt returns no blocks, so subsequent attempts are bypassed)
		auto context = CreateDefaultTestContext(2, 10, 8);
		context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2, 2, 0 });
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Success, result);
		AssertSync(context, 1);
		AssertDefaultMultiplePullRequest(*context.pChainApi, { Height(13), Height(15), Height(17) });
	}

	TEST(TEST_CLASS, FailedInteractionIfBlocksFromReturnsException) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		context.pChainApi->setError(MockChainApi::EntryPoint::Blocks_From);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		context.assertNoCalls();
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(TEST_CLASS, ReturnsNotReadyFutureWhenPullingBlocks) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
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
					test::GenerateVerifiableBlockAtHeight(Default_Height));
			context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2 });
			context.Config.MaxRollbackBlocks = 9;
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

	TEST(TEST_CLASS, SuccessfulInteractionIfContainerIsNeitherFullNorDirty) {
		// Arrange: by default the container max size is large enough to pass many sync rounds without being full
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// Act:
		for (auto i = 0u; i < 10u; i++)
			interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		AssertSync(context, 10);

		auto i = 0u;
		ASSERT_EQ(10u, context.pChainApi->blocksFromRequests().size());
		for (auto result : interactionResults) {
			EXPECT_EQ(NodeInteractionResult::Success, result);
			AssertRequestHeight(context, i, Default_Height + Height(2 * i));
			++i;
		}
	}

	TEST(TEST_CLASS, NeutralInteractionIfContainerIsFull) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		context.Config.MaxChainBytesPerSyncAttempt = sizeof(Block) / 3 - 1;
		auto synchronizer = CreateSynchronizer(context);

		// Act: second call is short circuited since the container is full
		auto result1 = synchronizer(*context.pChainApi).get();
		auto result2 = synchronizer(*context.pChainApi).get();

		// Assert:
		EXPECT_EQ(result1, NodeInteractionResult::Success);
		EXPECT_EQ(result2, NodeInteractionResult::Neutral);
		AssertSync(context, 1);

		AssertRequestHeights(context, { Default_Height });
	}

	TEST(TEST_CLASS, SuccessfulInteractionIfPreviouslyFullContainerIsNoLongerFull) {
		// Arrange: the container's max size is set to 3 * MaxChainBytesPerSyncAttempt = 3 * sizeof(Block)
		//          that means the container is full after 2 syncs (2 blocks per sync)
		auto context = CreateTestContextForUnprocessedElementTests();
		context.Config.MaxChainBytesPerSyncAttempt = sizeof(Block);
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// - sucessful since container is not full
		interactionResults.push_back(synchronizer(*context.pChainApi).get());
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// - neutral since container is full
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for first element has finished
		context.ProcessingComplete(1, CreateContinueResult());

		// Act: since an unprocessed element was removed, the container is no longer full
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral,
			NodeInteractionResult::Success
		};

		AssertSync(context, 3);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2), Default_Height + Height(4) });
	}

	TEST(TEST_CLASS, NeutralInteractionIfContainerIsDirty) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;
		interactionResults.push_back(synchronizer(*context.pChainApi).get());
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for first element has finished unsuccessfully making the container dirty
		context.ProcessingComplete(1, CreateAbortResult());

		// Act: neutral because container is dirty
		interactionResults.push_back(synchronizer(*context.pChainApi).get());
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral,
			NodeInteractionResult::Neutral
		};

		AssertSync(context, 2);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2) });
	}

	TEST(TEST_CLASS, DirtyContainerIsCleanAfterAllElementsHaveBeenRemoved) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;
		interactionResults.push_back(synchronizer(*context.pChainApi).get());
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for first element has finished unsuccessfully making the container dirty
		context.ProcessingComplete(1, CreateAbortResult());

		// - neutral because container is dirty
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// - signal processing for second element has finished. The container is empty and thus clean again
		context.ProcessingComplete(2, CreateAbortResult());

		// Act: successful because container is clean
		interactionResults.push_back(synchronizer(*context.pChainApi).get());

		// Assert:
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral,
			NodeInteractionResult::Success
		};

		AssertSync(context, 3);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		// - last request should be Default_Height because container is empty again
		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2), Default_Height });
	}

	TEST(TEST_CLASS, PendingBlocksAreMarkedAsDirtyIfRootBlocksAreDirty) {
		// Arrange: non-deterministic because of dependency on delay
		test::RunNonDeterministicTest("closes socket test", [](auto i) {
			// Arrange:
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);
			std::vector<NodeInteractionResult> interactionResults;

			// Act: start an (immediate) request
			interactionResults.push_back(synchronizer(*context.pChainApi).get());

			// - start a delayed request
			context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10 * i));
			auto syncFuture = synchronizer(*context.pChainApi);

			// - signal processing for first element has finished unsuccessfully making the container dirty
			context.ProcessingComplete(1, CreateAbortResult());

			// - wait for the delayed request (it must still be pending in order for this test to give a deterministic result)
			if (syncFuture.is_ready())
				return false;

			interactionResults.push_back(syncFuture.get());

			// Assert:
			std::vector<NodeInteractionResult> expectedInteractionResults{
				NodeInteractionResult::Success,
				NodeInteractionResult::Neutral
			};

			// - only one range was forwarded to the consumer because the second was rejected
			AssertSync(context, 1);
			EXPECT_EQ(expectedInteractionResults, interactionResults);

			AssertRequestHeights(context, { Default_Height, Default_Height + Height(2) });
			return true;
		});
	}

	TEST(TEST_CLASS, MultiplePendingSyncsAreNotAllowedAtOnce) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// Act: start two delayed requests
		context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
		auto syncFuture1 = synchronizer(*context.pChainApi);
		auto syncFuture2 = synchronizer(*context.pChainApi);

		// - wait for the delayed requests
		interactionResults.push_back(syncFuture1.get());
		interactionResults.push_back(syncFuture2.get());

		// Assert: only one outstanding sync is allowed at a time
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral
		};

		// - only one range was forwarded to the consumer because the second was bypassed
		AssertSync(context, 1);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		// - only one request was made because the second was bypassed
		AssertRequestHeights(context, { Default_Height });
	}

	TEST(TEST_CLASS, ThrowsIfCompletionHandlerIsCalledWithUnknownElementId) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		auto resultFuture1 = synchronizer(*context.pChainApi);

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
			std::vector<NodeInteractionResult> interactionResults;

			// Act: set an exception and sync
			context.pChainApi->setError(errorPoint);
			interactionResults.push_back(synchronizer(*context.pChainApi).get());

			// - clear the exception and sync
			context.pChainApi->setError(MockChainApi::EntryPoint::None);
			interactionResults.push_back(synchronizer(*context.pChainApi).get());

			// Assert: the first sync failed but the second succeeded
			std::vector<NodeInteractionResult> expectedInteractionResults{
				NodeInteractionResult::Failure,
				NodeInteractionResult::Success
			};

			AssertSync(context, 1);
			EXPECT_EQ(expectedInteractionResults, interactionResults);
			AssertRequestHeights(context, expectedRequestHeights);
		}
	}

	TEST(TEST_CLASS, CanRecoverAfterFailedCompareChainsOperation) {
		// Assert: only one blocks-from request was made because the chain comparison failed
		AssertSyncRecoverablityAfterChainApiException(MockChainApi::EntryPoint::Chain_Info, { Default_Height });
	}

	TEST(TEST_CLASS, CanRecoverAfterFailedBlocksFromOperation) {
		// Assert: two blocks-from requests were made beacuse the the chain comparison succeeded
		AssertSyncRecoverablityAfterChainApiException(MockChainApi::EntryPoint::Blocks_From, { Default_Height, Default_Height });
	}

	// endregion

	// region clean shutdown

	TEST(TEST_CLASS, SynchronizerFutureCanCompleteAfterSynchronizerIsDestroyed) {
		// Arrange:
		thread::future<NodeInteractionResult> future;
		auto context = CreateDefaultTestContext(9, 10);
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
